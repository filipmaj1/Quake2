/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// sv_world.cpp -- world query functions

#include "server.h"
#include "cmodel.h"


/*
===============================================================================

ENTITY AREA CHECKING

FIXME: this use of "area" is different from the bsp file use
===============================================================================
*/

#define EDICT_FROM_AREA(a)		((edict_t*) ( (byte*)a - (int)&((edict_t*)NULL) ->area ) )

struct areanode_t
{
	int		axis;				// -1 = leaf node
	float	dist;
	areanode_t *children[2];
	areanode_t *parent;
	int		numTrigEdicts;
	link_t	trigEdicts;
	int		numSolidEdicts;
	link_t	solidEdicts;
};

#define	AREA_DEPTH	6
#define	AREA_NODES	(1 << (AREA_DEPTH + 1))		// (1<<AREA_DEPTH) for nodes and same count for leafs

static areanode_t areaNodes[AREA_NODES];
static int		numAreaNodes;					// used only for creation of area tree


struct entityHull_t
{
	areanode_t *area;
	edict_t	*owner;
	cmodel_t *model;
	CVec3	center;
	float	radius;
	CAxis	axis;
	CBox	bounds;								// for alias models it is equal to client prediction code bbox
};

static entityHull_t ents[MAX_EDICTS];


/*---------------------------------------------------------------*/


static CBox		area_bounds;
static edict_t	**area_list;
static int		area_count, area_maxcount;
static int		area_type;

int SV_HullForEntity (edict_t *ent);


// ClearLink is used for new headnodes
static void ClearLink (link_t *l)
{
	l->prev = l->next = l;
}

static void RemoveLink (link_t *l)
{
	l->next->prev = l->prev;
	l->prev->next = l->next;
}

static void InsertLinkBefore (link_t *l, link_t *before)
{
	l->next = before;
	l->prev = before->prev;
	l->prev->next = l;
	l->next->prev = l;
}

/*
===============
SV_CreateAreaNode

Builds a uniformly subdivided tree for the given world size
===============
*/
static areanode_t *SV_CreateAreaNode (int depth, const CBox &bounds)
{
	areanode_t &anode = areaNodes[numAreaNodes++];

	ClearLink (&anode.trigEdicts);
	ClearLink (&anode.solidEdicts);

	if (depth == AREA_DEPTH)
	{
		anode.axis = -1;
		anode.children[0] = anode.children[1] = NULL;
		return &anode;
	}

	float f0 = bounds.maxs[0] - bounds.mins[0];
	float f1 = bounds.maxs[1] - bounds.mins[1];
	float f2 = bounds.maxs[2] - bounds.mins[2];
	if (f0 > f1)
	{
		anode.axis = f0 > f2 ? 0 : 2;
	}
	else
	{
		anode.axis = f1 > f2 ? 1 : 2;
	}

	// split bounds into 2 identical by volume sub-bounds
	anode.dist = (bounds.maxs[anode.axis] + bounds.mins[anode.axis]) / 2;
	CBox bounds1 = bounds;
	CBox bounds2 = bounds;
	bounds1.maxs[anode.axis] = bounds2.mins[anode.axis] = anode.dist;

	anode.children[0] = SV_CreateAreaNode (depth+1, bounds1);
	anode.children[1] = SV_CreateAreaNode (depth+1, bounds2);
	anode.children[0]->parent = anode.children[1]->parent = &anode;	// NULL for root, because of memset(..,0,..)

	return &anode;
}

/*
===============
SV_ClearWorld

===============
*/
void SV_ClearWorld (void)
{
	memset (areaNodes, 0, sizeof(areaNodes));
	memset (ents, 0, sizeof(ents));
	numAreaNodes = 0;
	if (!sv.models[1]) return;		// map is not yet loaded (check [1], not [0] ...)
	SV_CreateAreaNode (0, sv.models[1]->bounds);
}


/*
===============
SV_UnlinkEdict

===============
*/
void SV_UnlinkEdict (edict_t *ent)
{
	guard(SV_UnlinkEdict);

	if (!ent->area.prev) return;			// not linked
	RemoveLink (&ent->area);
	ent->area.prev = ent->area.next = NULL;

	entityHull_t &ex = ents[NUM_FOR_EDICT(ent)];
	areanode_t *node = ex.area;
	if (ent->solid == SOLID_TRIGGER)
		while (node)
		{
			node->numTrigEdicts++;
			node = node->parent;
		}
	else
		while (node)
		{
			node->numSolidEdicts++;
			node = node->parent;
		}
	ex.area = NULL;

	unguard;
}


/*
===============
SV_LinkEdict

===============
*/
#define MAX_TOTAL_ENT_LEAFS		128

void SV_LinkEdict (edict_t *ent)
{
	guard(SV_LinkEdict);

	int		i, j, k;

	if (ent->area.prev)
		SV_UnlinkEdict (ent);	// unlink from old position (i.e. relink edict)

	if (ent == ge->edicts)
		return;					// don't add the world

	if (!ent->inuse)
		return;

	entityHull_t &ex = ents[NUM_FOR_EDICT(ent)];
	ex.owner = ent;
	ex.axis.FromAngles (ent->s.angles);

	// set the size
	VectorSubtract (ent->bounds.maxs, ent->bounds.mins, ent->size);

	// encode the size into the entity_state for client prediction
	if (ent->solid == SOLID_BBOX)
	{
		// assume that x/y are equal and symetric
		i = appRound (ent->bounds.maxs[0] / 8);
		// z is not symetric
		j = appRound (-ent->bounds.mins[2] / 8);
		// and z maxs can be negative...
		k = appRound ((ent->bounds.maxs[2] + 32) / 8);
		// original Q2 have bounded i/j/k/ with lower margin==1 (for client prediction only); this will
		// produce incorrect collision test when bbox mins/maxs is (0,0,0)
		i = bound(i, 0, 31);		// mins/maxs[0,1] range is -248..0/0..248
		j = bound(j, 0, 31);		// mins[2] range is [-248..0]
		k = bound(k, 0, 63);		// maxs[2] range is [-32..472]

		// if SVF_DEADMONSTER, s.solid should be 0
		ent->s.solid = (ent->svflags & SVF_DEADMONSTER) ? 0 : (k<<10) | (j<<5) | i;

		i *= 8;
		j *= 8;
		k *= 8;
		ex.bounds.mins.Set (-i, -i, -j);
		ex.bounds.maxs.Set (i, i, k - 32);
		ex.bounds.GetCenter (ex.center);
		VectorAdd (ex.center, ent->s.origin, ex.center);
		ex.model = NULL;
		ex.radius = VectorDistance (ex.bounds.maxs, ex.bounds.mins) / 2;
	}
	else if (ent->solid == SOLID_BSP)
	{
		ex.model = sv.models[ent->s.modelindex];
		if (!ex.model) Com_DropError ("MOVETYPE_PUSH with a non bsp model");
		CVec3	v;
		ex.model->bounds.GetCenter (v);
		UnTransformPoint (ent->s.origin, ex.axis, v, ex.center);
		ex.radius = ex.model->radius;

		ent->s.solid = 31;		// a SOLID_BBOX will never create this value (mins=(-248,-248,0) maxs=(248,248,-32))
	}
	else
		ent->s.solid = 0;

	// set the abs box
	if (ent->solid == SOLID_BSP && (ent->s.angles[0] || ent->s.angles[1] || ent->s.angles[2]))
	{
		// expand for rotation
		for (i = 0; i < 3 ; i++)
		{
			ent->absBounds.mins[i] = ex.center[i] - ex.radius;
			ent->absBounds.maxs[i] = ex.center[i] + ex.radius;
		}
	}
	else
	{	// normal
		VectorAdd (ent->s.origin, ent->bounds.mins, ent->absBounds.mins);
		VectorAdd (ent->s.origin, ent->bounds.maxs, ent->absBounds.maxs);
	}

	// because movement is clipped an epsilon away from an actual edge,
	// we must fully check even when bounding boxes don't quite touch
	for (i = 0; i < 3; i++)
	{
		ent->absBounds.mins[i] -= 1;
		ent->absBounds.maxs[i] += 1;
	}

	// link to PVS leafs
	ent->num_clusters = 0;
	ent->areanum = 0;
	ent->areanum2 = 0;

	// get all leafs, including solids
	int leafs[MAX_TOTAL_ENT_LEAFS];
	int topnode;
	int num_leafs = CM_BoxLeafnums (ent->absBounds, ARRAY_ARG(leafs), &topnode);

	// set areas
	int clusters[MAX_TOTAL_ENT_LEAFS];
	for (i = 0; i < num_leafs; i++)
	{
		clusters[i] = CM_LeafCluster (leafs[i]);
		int area = CM_LeafArea (leafs[i]);
		if (area)
		{	// doors may legally straggle two areas,
			// but nothing should evern need more than that
			if (ent->areanum && ent->areanum != area)
			{
				if (ent->areanum2 && ent->areanum2 != area && sv.state == ss_loading)
					Com_DPrintf ("Object touching 3 areas at %g %g %g\n", VECTOR_ARG(ent->absBounds.mins));
				ent->areanum2 = area;
			}
			else
				ent->areanum = area;
		}
	}

	if (num_leafs >= MAX_TOTAL_ENT_LEAFS)
	{	// assume we missed some leafs, and mark by headnode
		ent->num_clusters = -1;
		ent->headnode = topnode;
	}
	else
	{
		ent->num_clusters = 0;
		for (i = 0; i < num_leafs; i++)
		{
			if (clusters[i] == -1)
				continue;		// not a visible leaf
			for (j = 0; j < i; j++)
				if (clusters[j] == clusters[i])
					break;
			if (j == i)
			{
				if (ent->num_clusters == MAX_ENT_CLUSTERS)
				{	// assume we missed some leafs, and mark by headnode
					ent->num_clusters = -1;
					ent->headnode = topnode;
					break;
				}

				ent->clusternums[ent->num_clusters++] = clusters[i];
			}
		}
	}

	// if first time, make sure old_origin is valid
	if (!ent->linkcount)
		ent->s.old_origin = ent->s.origin;
	ent->linkcount++;

	if (ent->solid == SOLID_NOT)
		return;

	// find the first node that the ent's box crosses
	areanode_t *node = areaNodes;
	while (node->axis != -1)
	{
		if (ent->absBounds.mins[node->axis] > node->dist)
			node = node->children[0];
		else if (ent->absBounds.maxs[node->axis] < node->dist)
			node = node->children[1];
		else
			break;		// crosses the node
	}

	// link it in
	areanode_t *node2 = node;
	if (ent->solid == SOLID_TRIGGER)
	{
		InsertLinkBefore (&ent->area, &node->trigEdicts);
		while (node2)
		{
			node2->numTrigEdicts++;
			node2 = node2->parent;
		}
	}
	else
	{
		InsertLinkBefore (&ent->area, &node->solidEdicts);
		while (node2)
		{
			node2->numSolidEdicts++;
			node2 = node2->parent;
		}
	}
	ex.area = node;

	unguard;
}


/*
====================
SV_AreaEdicts

====================
*/
static void SV_AreaEdicts_r (areanode_t *node)
{
	link_t	*l, *next, *start;

	// touch linked edicts
	if (area_type == AREA_SOLID)
	{
		if (!node->numSolidEdicts) return;
		start = &node->solidEdicts;
	}
	else
	{
		if (!node->numTrigEdicts) return;
		start = &node->trigEdicts;
	}

	for (l = start->next; l != start; l = next)
	{
		next = l->next;
		edict_t *check = EDICT_FROM_AREA(l);

		if (check->solid == SOLID_NOT)
			continue;		// deactivated
		if (check->absBounds.mins[2] > area_bounds.maxs[2] || check->absBounds.maxs[2] < area_bounds.mins[2] ||
			check->absBounds.mins[0] > area_bounds.maxs[0] || check->absBounds.maxs[0] < area_bounds.mins[0] ||
			check->absBounds.mins[1] > area_bounds.maxs[1] || check->absBounds.maxs[1] < area_bounds.mins[1])
			continue;		// not touching

		if (area_count == area_maxcount)
		{
			Com_WPrintf ("SV_AreaEdicts: MAXCOUNT\n");
			return;
		}

		area_list[area_count++] = check;
	}

	if (node->axis == -1)
		return;				// terminal node

	if (area_bounds.maxs[node->axis] > node->dist) SV_AreaEdicts_r (node->children[0]);
	if (area_bounds.mins[node->axis] < node->dist) SV_AreaEdicts_r (node->children[1]);
}


int SV_AreaEdicts (const CVec3 &mins, const CVec3 &maxs, edict_t **list, int maxcount, int areatype)
{
	guard(SV_AreaEdicts);
	area_bounds.mins = mins;
	area_bounds.maxs = maxs;
	area_list = list;
	area_count = 0;
	area_maxcount = maxcount;
	area_type = areatype;

	SV_AreaEdicts_r (areaNodes);

	return area_count;
	unguard;
}


//===========================================================================

/*
=============
SV_PointContents
=============
*/
int SV_PointContents (const CVec3 &p)
{
	guard(SV_PointContents);

	// get base contents from world
	unsigned contents = CM_PointContents (p, sv.models[1]->headnode);

	edict_t	*list[MAX_EDICTS];
	int num = SV_AreaEdicts (p, p, ARRAY_ARG(list), AREA_SOLID);

	for (int i = 0; i < num; i++)
	{
		edict_t *edict = list[i];
		entityHull_t *ent = &ents[NUM_FOR_EDICT(edict)];

		unsigned c2;
		if (ent->model)
			c2 = CM_TransformedPointContents (p, ent->model->headnode, edict->s.origin, ent->axis);
		else
			c2 = CM_TransformedPointContents (p, CM_HeadnodeForBox (ent->bounds), edict->s.origin, nullVec3);
		contents |= c2;
	}
	return contents;

	unguard;
}


//===========================================================================

/*
====================
SV_ClipMoveToEntities

====================
*/
static void SV_ClipMoveToEntities (trace_t &tr, const CVec3 &start, const CVec3 &mins, const CVec3 &maxs, const CVec3 &end,
	edict_t *passedict, int contentmask)
{
	guard(SV_ClipMoveToEntities);

	if (tr.allsolid) return;

	int		i;

	CVec3 amins, amaxs;
	for (i = 0; i < 3; i++)
	{
		if (start[i] < end[i])
		{
			amins[i] = start[i] + mins[i];
			amaxs[i] = end[i] + maxs[i];
		}
		else
		{
			amins[i] = end[i] + mins[i];
			amaxs[i] = start[i] + maxs[i];
		}
	}
	edict_t	*list[MAX_EDICTS];
	int num = SV_AreaEdicts (amins, amaxs, ARRAY_ARG(list), AREA_SOLID);
	if (!num) return;

	float b1 = dot (mins, mins);
	float b2 = dot (maxs, maxs);
	float t = max(b1, b2);
	float traceWidth = SQRTFAST(t);
	CVec3 traceDir;
	VectorSubtract (end, start, traceDir);
	float traceLen = traceDir.Normalize () + traceWidth;

	for (i = 0; i < num; i++)
	{
		edict_t *edict = list[i];
		entityHull_t &ent = ents[NUM_FOR_EDICT(edict)];
//		if (!ent->linked) continue;

		if (edict->solid == SOLID_NOT || edict == passedict) continue;
		if (passedict)
		{
		 	if (edict->owner == passedict)
				continue;	// don't clip against own missiles
			if (passedict->owner == edict)
				continue;	// don't clip against owner
		}
		if (!(contentmask & CONTENTS_DEADMONSTER) && (edict->svflags & SVF_DEADMONSTER))
			continue;

		CVec3 center;
		VectorSubtract (ent.center, start, center);
		// check position of point projection on line
		float entPos = dot (center, traceDir);
		if (entPos < -traceWidth - ent.radius || entPos > traceLen + ent.radius)
			continue;		// too near / too far

		// check distance between point and line
		CVec3 tmp;
		VectorMA (center, -entPos, traceDir, tmp);
		float dist2 = dot (tmp, tmp);
		float dist0 = ent.radius + traceWidth;
		if (dist2 >= dist0 * dist0) continue;

		trace_t	trace;
		if (ent.model)
			CM_TransformedBoxTrace (trace, start, end, &mins, &maxs, ent.model->headnode, contentmask, edict->s.origin, ent.axis);
		else
			CM_TransformedBoxTrace (trace, start, end, &mins, &maxs, CM_HeadnodeForBox (ent.bounds), contentmask, edict->s.origin, nullVec3);

		if (trace.allsolid || trace.startsolid || trace.fraction < tr.fraction)
		{
			trace.ent = edict;
		 	if (tr.startsolid)
			{
				tr = trace;
				tr.startsolid = true;
			}
			else
				tr = trace;
		}
		else if (trace.startsolid)
			tr.startsolid = true;
		if (tr.allsolid) return;
	}

	unguard;
}


/*
==================
SV_Trace

Moves the given mins/maxs volume through the world from start to end.

Passedict and edicts owned by passedict are explicitly not checked.

==================
*/
//!! change argument order (for game, SV_TraceHook() used anyway)
void SV_Trace (trace_t &tr, const CVec3 &start, const CVec3 &mins, const CVec3 &maxs, const CVec3 &end, edict_t *passedict, int contentmask)
{
	guard(SV_Trace);

	// clip to world
	CM_BoxTrace (tr, start, end, &mins, &maxs, 0, contentmask);

	tr.ent = ge->edicts;
	if (!tr.fraction) return;		// blocked by the world

	// clip to other solid entities
	SV_ClipMoveToEntities (tr, start, mins, maxs, end, passedict, contentmask);

	unguard;
}
