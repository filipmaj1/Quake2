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

// r_main.c
#include "gl_local.h"


/*----------------- rudiments ---------------------------*/
//void ( APIENTRY * glColorTableEXT)( int, int, int, int, int, const void * );
//void ( APIENTRY * glPointParameterfEXT)( GLenum param, GLfloat value );
//void ( APIENTRY * glPointParameterfvEXT)( GLenum param, const GLfloat *value );

/*-------------------------------------------------------*/

void R_Clear (void);

viddef_t	vid;

#ifdef DYNAMIC_REF
refImport_t	ri;
#endif
static int ref_flags;

model_t		*r_worldmodel;

float		gldepthmin, gldepthmax;

glconfig_t	gl_config;
glstate_t	gl_state;
gl_speeds_t	gl_speeds;

image_t		*r_notexture;		// use for bad textures
image_t		*r_particletexture;	// little dot for particles

entity_t	*currententity;
model_t		*currentmodel;

cplane_t	frustum[4];

int			r_visframecount;	// bumped when going to a new PVS
int			r_framecount;		// used for dlight push checking

int			c_brush_polys, c_alias_polys;

float		v_blend[4];			// final blending color

void GL_Strings_f( void );

//
// view origin
//
vec3_t	vup;
vec3_t	vpn;
vec3_t	vright;
vec3_t	r_origin;

float	r_world_matrix[16];
float	r_base_world_matrix[16];

//
// screen size info
//
refdef_t	r_newrefdef;

int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

cvar_t	*r_norefresh;
cvar_t	*r_drawentities;
static cvar_t *r_drawworld;

static cvar_t	*r_speeds;
cvar_t	*r_ignorehwgamma;

cvar_t	*r_novis;
cvar_t	*gl_frustumCull;
cvar_t	*r_lerpmodels;
cvar_t	*r_lefthand;

float	gl_lightlevel;

cvar_t	*gl_nosubimage;
cvar_t	*gl_allow_software;

cvar_t	*gl_vertex_arrays;

cvar_t	*gl_particle_min_size;
cvar_t	*gl_particle_max_size;
cvar_t	*gl_particle_size;
cvar_t	*gl_particle_att_a;
cvar_t	*gl_particle_att_b;
cvar_t	*gl_particle_att_c;

//cvar_t	*gl_ext_palettedtexture;
//cvar_t	*gl_ext_pointparameters;

cvar_t	*gl_logFile;
cvar_t	*gl_bitdepth;
cvar_t  *gl_driver;
cvar_t	*gl_shadows;
cvar_t	*gl_mode;
cvar_t	*gl_dynamic;
cvar_t	*gl_dlightBacks;
cvar_t  *gl_monolightmap;
cvar_t	*gl_modulate;
cvar_t	*gl_nobind;
cvar_t	*gl_round_down;
cvar_t	*gl_picmip;
cvar_t	*gl_skymip;
cvar_t	*gl_showtris;
cvar_t	*gl_ztrick;
cvar_t	*gl_finish;
cvar_t	*gl_clear;
cvar_t	*gl_cull;
cvar_t	*gl_polyblend;
cvar_t	*gl_flashblend;
cvar_t	*gl_playermip;
cvar_t  *gl_saturatelighting;
cvar_t	*gl_swapinterval;
cvar_t	*gl_texturemode;
cvar_t	*gl_texturealphamode;
cvar_t	*gl_texturesolidmode;
cvar_t	*gl_lockpvs;
cvar_t  *gl_zmax;
cvar_t  *gl_winrefl;

cvar_t	*gl_3dlabs_broken;

/*
=================
R_CullBox

Returns true if the box is completely outside the frustom
=================
*/
bool R_CullBox (vec3_t mins, vec3_t maxs)
{
	int		i;

	if (!gl_frustumCull->integer)
		return false;

	for (i=0 ; i<4 ; i++)
		if ( BOX_ON_PLANE_SIDE(mins, maxs, &frustum[i]) == 2)
			return true;
	return false;
}


void R_RotateForEntity (entity_t *e)
{
	glTranslatef (e->origin[0],  e->origin[1],  e->origin[2]);

	glRotatef (e->angles[1],  0, 0, 1);
	glRotatef (-e->angles[0],  0, 1, 0);
	glRotatef (-e->angles[2],  1, 0, 0);
}

/*
=============================================================

  SPRITE MODELS

=============================================================
*/


/*
=================
R_DrawSpriteModel

=================
*/
void R_DrawSpriteModel (entity_t *e)
{
	float	alpha;
	vec3_t	point, up, down;
	dsprframe_t	*frame;
	dsprite_t *psprite;

	// don't even bother culling, because it's just a single
	// polygon without a surface cache

	psprite = (dsprite_t *)currentmodel->extradata;

	e->frame %= psprite->numframes;

	frame = &psprite->frames[e->frame];

	if (!currentmodel->skins[e->frame]) return; // no skin (bad model?)

	if (e->flags & RF_TRANSLUCENT)
		alpha = e->alpha;
	else
		alpha = 1;

	glColor4f( 1, 1, 1, alpha );

	GL_Bind(currentmodel->skins[e->frame]->texnum);

	GL_TexEnv (GL_MODULATE);

	if (alpha == 1.0)
	{
		// following 2 lines required to draw 2 sprites
		glEnable (GL_ALPHA_TEST);
		glAlphaFunc (GL_GEQUAL, 0.05);
	}
	else
	{
		glDisable (GL_ALPHA_TEST);
	}

	glEnable (GL_BLEND); //?? required for sprites with transparency
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//??

	VectorMA (e->origin, frame->height - frame->origin_y, vup, up);
	VectorMA (e->origin, -frame->origin_y, vup, down);

	glBegin (GL_QUADS);

	glTexCoord2f (0, 1);
	VectorMA (down, -frame->origin_x, vright, point);
	glVertex3fv (point);

	glTexCoord2f (0, 0);
	VectorMA (up, -frame->origin_x, vright, point);
	glVertex3fv (point);

	glTexCoord2f (1, 0);
	VectorMA (up, frame->width - frame->origin_x, vright, point);
	glVertex3fv (point);

	glTexCoord2f (1, 1);
	VectorMA (down, frame->width - frame->origin_x, vright, point);
	glVertex3fv (point);

	glEnd ();

	// restore default state
	glDisable (GL_ALPHA_TEST);
	glAlphaFunc (GL_GEQUAL, 0.666);
	glDisable (GL_BLEND);
	glColor4f (1, 1, 1, 1);

	GL_TexEnv( GL_REPLACE );
}

//==================================================================================

/*
=============
R_DrawNullModel
=============
*/
void R_DrawNullModel (void)
{
	vec3_t	shadelight;
	int		i;

	if ( currententity->flags & RF_FULLBRIGHT )
		shadelight[0] = shadelight[1] = shadelight[2] = 1.0F;
	else
		R_LightPoint (currententity->origin, shadelight);

	glPushMatrix ();
	R_RotateForEntity (currententity);

	glDisable (GL_TEXTURE_2D);
	glColor3fv (shadelight);

	glBegin (GL_TRIANGLE_FAN);
	glVertex3f (0, 0, -16);
	for (i=0 ; i<=4 ; i++)
		glVertex3f (16*cos(i*M_PI/2), 16*sin(i*M_PI/2), 0);
	glEnd ();

	glBegin (GL_TRIANGLE_FAN);
	glVertex3f (0, 0, 16);
	for (i=4 ; i>=0 ; i--)
		glVertex3f (16*cos(i*M_PI/2), 16*sin(i*M_PI/2), 0);
	glEnd ();

	glColor3f (1,1,1);
	glPopMatrix ();
	glEnable (GL_TEXTURE_2D);
}

/*
=============
R_DrawEntitiesOnList
=============
*/
void R_DrawEntitiesOnList (void)
{
	int		i;
	beam_t	*b;

	if (!r_drawentities->integer)
		return;

	// draw non-transparent first
	for (i = 0; i < r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;

		if (currententity->flags & RF_TRANSLUCENT || !currentmodel || currentmodel->flags & CMODEL_ALPHA)
			continue;	// solid

		if (!currentmodel)
		{
//			DrawTextLeft("NULL", RGB(1,1,1));//!!
			R_DrawNullModel ();
			continue;
		}
//		DrawTextLeft(va("\"%s\"",currentmodel->name), RGB(1,1,1));//!!
		switch (currentmodel->type)
		{
		case mod_alias:
			R_DrawAliasModel (currententity);
			break;
		case mod_brush:
		case mod_hl_brush:
			R_DrawBrushModel (currententity);
			break;
		case mod_sprite:
			R_DrawSpriteModel (currententity);
			break;
		default:
			Com_DropError ("Bad modeltype");
			break;
		}
	}

	// draw transparent entities
	// we could sort these if it ever becomes a problem...
	glDepthMask (0);		// no z writes
	for (i = 0; i < r_newrefdef.num_entities; i++)
	{
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;

		if (!(currententity->flags & RF_TRANSLUCENT || !currentmodel || currentmodel->flags & CMODEL_ALPHA))
			continue;	// solid

		if (!currentmodel)
		{
//			DrawTextLeft("NULL2", RGB(1,1,1));//!!
			R_DrawNullModel ();
			continue;
		}
//		DrawTextLeft(va("\"%s\"2",currentmodel->name), RGB(1,1,1));//!!
		switch (currentmodel->type)
		{
		case mod_alias:
			R_DrawAliasModel (currententity);
			break;
		case mod_brush:
		case mod_hl_brush:
			R_DrawBrushModel (currententity);
			break;
		case mod_sprite:
			R_DrawSpriteModel (currententity);
			break;
		default:
			Com_DropError ("Bad modeltype");
			break;
		}
	}
	for (b = r_newrefdef.beams; b; b = b->next)
		R_DrawBeam (b);

	glDepthMask (1);		// back to writing
}

/*
** GL_DrawParticles
**
*/
void GL_DrawParticles (particle_t *particles, const unsigned colortable[768])
{
	const particle_t *p;
	vec3_t			up, right;
	float			scale;
	byte			color[4];

	GL_Bind(r_particletexture->texnum);
	glDepthMask (GL_FALSE);		// no z buffering
	glEnable (GL_BLEND);
	GL_TexEnv (GL_MODULATE);
	glBegin (GL_TRIANGLES);

	VectorScale (vup, 1.5, up);
	VectorScale (vright, 1.5, right);

	for (p = particles; p; p = p->next)
	{
		// hack a scale up to keep particles from disapearing
		scale = ( p->org[0] - r_origin[0] ) * vpn[0] +
			    ( p->org[1] - r_origin[1] ) * vpn[1] +
			    ( p->org[2] - r_origin[2] ) * vpn[2];

		if (scale < 20)
			scale = 1;
		else
			scale = 1 + scale * 0.004;

		*(int *)color = colortable[p->color];
		color[3] = p->alpha * 255;
		glColor4ubv (color);

		glTexCoord2f (0.0625, 0.0625);
		glVertex3fv (p->org);

		glTexCoord2f (1.0625, 0.0625);
		glVertex3f (p->org[0] + up[0] * scale,
			         p->org[1] + up[1] * scale,
					 p->org[2] + up[2] * scale);

		glTexCoord2f (0.0625, 1.0625);
		glVertex3f (p->org[0] + right[0] * scale,
			         p->org[1] + right[1] * scale,
					 p->org[2] + right[2] * scale);
	}

	glEnd ();
	glDisable (GL_BLEND);
	glColor4f (1,1,1,1);
	glDepthMask (1);		// back to normal Z buffering
	GL_TexEnv (GL_REPLACE);
}

/*
===============
R_DrawParticles
===============
*/
void R_DrawParticles (void)
{
/*	if (gl_ext_pointparameters->integer && glPointParameterfEXT)
	{
		int i;
		unsigned char color[4];
		const particle_t *p;

		glDepthMask (GL_FALSE);
		glEnable (GL_BLEND);
		glDisable (GL_TEXTURE_2D);

		glPointSize (gl_particle_size->value);

		glBegin (GL_POINTS);
//		glBegin (GL_LINES);//(GL_POINTS);
		for (i = 0, p = r_newrefdef.particles; i < r_newrefdef.num_particles; i++, p++)
		{
			*(int *)color = d_8to24table[p->color];
			color[3] = p->alpha*255;

			glColor4ubv (color);

			glVertex3fv (p->origin);
//			glVertex3fv (p->prev);
		}
		glEnd();

		glDisable (GL_BLEND);
		glColor4f (1.0F, 1.0F, 1.0F, 1.0F);
		glDepthMask (GL_TRUE);
		glEnable (GL_TEXTURE_2D);

	}
	else
*/	{
		GL_DrawParticles (r_newrefdef.particles, d_8to24table);
	}
}

/*
============
R_PolyBlend
============
*/
void R_PolyBlend (void)
{
	if (!gl_polyblend->integer)
		return;
	if (!v_blend[3])
		return;

	glDisable (GL_ALPHA_TEST);
	glEnable (GL_BLEND);
	glDisable (GL_DEPTH_TEST);
	glDisable (GL_TEXTURE_2D);

	glLoadIdentity ();

	// FIXME: get rid of these
	glRotatef (-90,  1, 0, 0);	    // put Z going up
	glRotatef (90,  0, 0, 1);	    // put Z going up

	glColor4fv (v_blend);

	glBegin (GL_QUADS);

	glVertex3f (10, 100, 100);
	glVertex3f (10, -100, 100);
	glVertex3f (10, -100, -100);
	glVertex3f (10, 100, -100);
	glEnd ();

	glDisable (GL_BLEND);
	glEnable (GL_TEXTURE_2D);
	glEnable (GL_ALPHA_TEST);

	glColor4f(1,1,1,1);
}

//=======================================================================

int SignbitsForPlane (cplane_t *out)
{
	int	bits, j;

	// for fast box on planeside test

	bits = 0;
	for (j=0 ; j<3 ; j++)
	{
		if (out->normal[j] < 0)
			bits |= 1<<j;
	}
	return bits;
}


void R_SetFrustum (void)
{
	int		i;

#if 0
	/*
	** this code is wrong, since it presume a 90 degree FOV both in the
	** horizontal and vertical plane
	*/
	// front side is visible
	VectorAdd (vpn, vright, frustum[0].normal);
	VectorSubtract (vpn, vright, frustum[1].normal);
	VectorAdd (vpn, vup, frustum[2].normal);
	VectorSubtract (vpn, vup, frustum[3].normal);

	// we theoretically don't need to normalize these vectors, but I do it
	// anyway so that debugging is a little easier
	VectorNormalize( frustum[0].normal );
	VectorNormalize( frustum[1].normal );
	VectorNormalize( frustum[2].normal );
	VectorNormalize( frustum[3].normal );
#else
	// rotate VPN right by FOV_X/2 degrees
	RotatePointAroundVector( frustum[0].normal, vup, vpn, -(90-r_newrefdef.fov_x / 2 ) );
	// rotate VPN left by FOV_X/2 degrees
	RotatePointAroundVector( frustum[1].normal, vup, vpn, 90-r_newrefdef.fov_x / 2 );
	// rotate VPN up by FOV_X/2 degrees
	RotatePointAroundVector( frustum[2].normal, vright, vpn, 90-r_newrefdef.fov_y / 2 );
	// rotate VPN down by FOV_X/2 degrees
	RotatePointAroundVector( frustum[3].normal, vright, vpn, -( 90 - r_newrefdef.fov_y / 2 ) );
#endif

	for (i=0 ; i<4 ; i++)
	{
		frustum[i].type = PLANE_NON_AXIAL;
		frustum[i].dist = DotProduct (r_origin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane (&frustum[i]);
	}
}

//=======================================================================

/*
===============
R_SetupFrame
===============
*/
void R_SetupFrame (void)
{
	int i;
	mleaf_t	*leaf;

	r_framecount++;

	// build the transformation matrix for the given view angles
	VectorCopy (r_newrefdef.vieworg, r_origin);

	AngleVectors (r_newrefdef.viewangles, vpn, vright, vup);

	// current viewcluster
	if ( !( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) )
	{
		r_oldviewcluster = r_viewcluster;
		r_oldviewcluster2 = r_viewcluster2;
		leaf = Mod_PointInLeaf (r_origin, r_worldmodel);
		r_viewcluster = r_viewcluster2 = leaf->cluster;

		// check above and below so crossing solid water doesn't draw wrong
		if (!leaf->contents)
		{	// look down a bit
			vec3_t	temp;

			VectorCopy (r_origin, temp);
			temp[2] -= 16;
			leaf = Mod_PointInLeaf (temp, r_worldmodel);
			if ( !(leaf->contents & CONTENTS_SOLID) &&
				(leaf->cluster != r_viewcluster2) )
				r_viewcluster2 = leaf->cluster;
		}
		else
		{	// look up a bit
			vec3_t	temp;

			VectorCopy (r_origin, temp);
			temp[2] += 16;
			leaf = Mod_PointInLeaf (temp, r_worldmodel);
			if ( !(leaf->contents & CONTENTS_SOLID) &&
				(leaf->cluster != r_viewcluster2) )
				r_viewcluster2 = leaf->cluster;
		}
	}

	for (i=0 ; i<4 ; i++)
		v_blend[i] = r_newrefdef.blend[i];

	c_brush_polys = 0;
	c_alias_polys = 0;

	// clear out the portion of the screen that the NOWORLDMODEL defines
	if ( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
	{
		glEnable( GL_SCISSOR_TEST );
		glClearColor( 0.3, 0.3, 0.3, 1 );
		glScissor( r_newrefdef.x, vid.height - r_newrefdef.height - r_newrefdef.y, r_newrefdef.width, r_newrefdef.height );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glClearColor( 1, 0, 0.5, 0.5 );
		glDisable( GL_SCISSOR_TEST );
	}
}


void MYgluPerspective( GLdouble fovy, GLdouble aspect,
		     GLdouble zNear, GLdouble zFar )
{
	GLdouble xmin, xmax, ymin, ymax;

	ymax = zNear * tan( fovy * M_PI / 360.0 );
	ymin = -ymax;

	xmin = ymin * aspect;
	xmax = ymax * aspect;

	xmin += -( 2 * gl_state.camera_separation ) / zNear;
	xmax += -( 2 * gl_state.camera_separation ) / zNear;

	glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}


/*
=============
R_SetupGL
=============
*/
void R_SetupGL (void)
{
	float	screenaspect;
//	float	yfov;
	int		x, x2, y2, y, w, h;

	//
	// set up viewport
	//
	x = appFloor(r_newrefdef.x * vid.width / vid.width);
	x2 = appCeil((r_newrefdef.x + r_newrefdef.width) * vid.width / vid.width);
	y = appFloor(vid.height - r_newrefdef.y * vid.height / vid.height);
	y2 = appCeil(vid.height - (r_newrefdef.y + r_newrefdef.height) * vid.height / vid.height);

	w = x2 - x;
	h = y - y2;

	glViewport (x, y2, w, h);

	//
	// set up projection matrix
	//
	screenaspect = (float)r_newrefdef.width/r_newrefdef.height;
//	yfov = 2*atan((float)r_newrefdef.height/r_newrefdef.width)*180/M_PI;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	MYgluPerspective (r_newrefdef.fov_y,  screenaspect,  4,  gl_zmax->integer * 2.5); // old value = 4096

	glCullFace(GL_FRONT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity ();

	glRotatef (-90,  1, 0, 0);	    // put Z going up
	glRotatef (90,  0, 0, 1);	    // put Z going up
	glRotatef (-r_newrefdef.viewangles[2],  1, 0, 0);
	glRotatef (-r_newrefdef.viewangles[0],  0, 1, 0);
	glRotatef (-r_newrefdef.viewangles[1],  0, 0, 1);
	glTranslatef (-r_newrefdef.vieworg[0],  -r_newrefdef.vieworg[1],  -r_newrefdef.vieworg[2]);

//	if ( gl_state.camera_separation != 0 && gl_state.stereo_enabled )
//		glTranslatef ( gl_state.camera_separation, 0, 0 );

	glGetFloatv (GL_MODELVIEW_MATRIX, r_world_matrix);

	//
	// set drawing parms
	//
	if (gl_cull->integer)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_DEPTH_TEST);
}

/*
=============
R_Clear
=============
*/
void R_Clear (void)
{
	if (ref_flags & REF_CONSOLE_ONLY)
	{
		glClearColor (0,0,0,1);
		glClear (GL_COLOR_BUFFER_BIT);
		return;
	}

	if (gl_ztrick->integer)
	{
		static int trickframe;

		if (gl_clear->integer)
			glClear (GL_COLOR_BUFFER_BIT);

		trickframe++;
		if (trickframe & 1)
		{
			gldepthmin = 0;
			gldepthmax = 0.49999;
			glDepthFunc (GL_LEQUAL);
		}
		else
		{
			gldepthmin = 1;
			gldepthmax = 0.5;
			glDepthFunc (GL_GEQUAL);
		}
	}
	else
	{
		if (gl_clear->integer)
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		else
			glClear (GL_DEPTH_BUFFER_BIT);
		gldepthmin = 0;
		gldepthmax = 1;
		glDepthFunc (GL_LEQUAL);
	}

	glDepthRange (gldepthmin, gldepthmax);

}

void R_Flash( void )
{
	R_PolyBlend ();
}

/*
================
R_RenderView

r_newrefdef must be set before the first call
================
*/

void R_RenderView (refdef_t *fd)
{
	if (r_norefresh->integer)
		return;

	r_newrefdef = *fd;

	if (!r_worldmodel && !( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) )
		Com_DropError ("R_RenderView: NULL worldmodel");

	if (r_speeds->integer)
	{
		c_brush_polys = 0;
		c_alias_polys = 0;
	}

	R_PushDlights ();

	R_SetupFrame ();

	R_SetFrustum ();

	R_SetupGL ();

	R_MarkLeaves ();	// done here so we know if we're in water

	if (r_drawworld->integer) R_DrawWorld ();

	R_DrawEntitiesOnList ();

	R_RenderDlights ();

	R_DrawParticles ();

	R_DrawAlphaSurfaces ();

	R_Flash ();

	if (r_speeds->integer)
	{
		DrawTextRight (va("%4i wpoly %4i epoly %i tex %i lmaps",
			c_brush_polys, c_alias_polys, c_visible_textures, c_visible_lightmaps),
			RGB(1, 0.5, 0));
		if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
			DrawTextRight (va("%4i visleafs %4i frustleafs of %4i total",
				c_visibleleafs, c_leafsinfrustum, r_worldmodel->numleafs),
				RGB(1, 0.5, 0));
	}
}


void	R_SetGL2D (void)
{
	if (gl_screenshotName && gl_screenshotFlags & SHOT_NO_2D)
		GL_PerformScreenshot ();

	// set 2D virtual screen size
	glViewport (0,0, vid.width, vid.height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	glOrtho  (0, vid.width, vid.height, 0, -BIG_NUMBER, BIG_NUMBER);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity ();
	glDisable (GL_DEPTH_TEST);
	glDisable (GL_CULL_FACE);
	glDisable (GL_BLEND);
	glEnable (GL_ALPHA_TEST);
	glColor4f (1,1,1,1);
}

static void GL_DrawColoredStereoLinePair( float r, float g, float b, float y )
{
	glColor3f( r, g, b );
	glVertex2f( 0, y );
	glVertex2f( vid.width, y );
	glColor3f( 0, 0, 0 );
	glVertex2f( 0, y + 1 );
	glVertex2f( vid.width, y + 1 );
}

static void GL_DrawStereoPattern( void )
{
#if 0
	if ( !( gl_config.renderer & GL_RENDERER_INTERGRAPH ) )
		return;

	if ( !gl_state.stereo_enabled )
		return;

	R_SetGL2D();

	glDrawBuffer( GL_BACK_LEFT );

	for (int i = 0; i < 20; i++)
	{
		glBegin( GL_LINES );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 0 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 2 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 4 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 6 );
			GL_DrawColoredStereoLinePair( 0, 1, 0, 8 );
			GL_DrawColoredStereoLinePair( 1, 1, 0, 10);
			GL_DrawColoredStereoLinePair( 1, 1, 0, 12);
			GL_DrawColoredStereoLinePair( 0, 1, 0, 14);
		glEnd();

		GLimp_EndFrame();
	}
#endif
}


/*
====================
R_SetLightLevel

====================
*/
void R_SetLightLevel (void)
{
	vec3_t		shadelight;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// save off light value for server to look at (BIG HACK!)

	R_LightPoint (r_newrefdef.vieworg, shadelight);

	// pick the greatest component, which should be the same
	// as the mono value returned by software
	if (shadelight[0] > shadelight[1])
	{
		if (shadelight[0] > shadelight[2])
			gl_lightlevel = 150*shadelight[0];
		else
			gl_lightlevel = 150*shadelight[2];
	}
	else
	{
		if (shadelight[1] > shadelight[2])
			gl_lightlevel = 150*shadelight[1];
		else
			gl_lightlevel = 150*shadelight[2];
	}

}

/*
@@@@@@@@@@@@@@@@@@@@@
R_RenderFrame

@@@@@@@@@@@@@@@@@@@@@
*/
void R_RenderFrame (refdef_t *fd)
{
	R_RenderView (fd);
	R_SetLightLevel ();
	R_SetGL2D ();
	R_DrawTexts ();
}


void R_Register( void )
{
CVAR_BEGIN(vars)
	CVAR_FULL(&r_lefthand, "hand", "0", CVAR_USERINFO|CVAR_ARCHIVE),
	CVAR_VAR(r_norefresh, 0, 0),
	CVAR_VAR(r_drawentities, 1, 0),
	CVAR_VAR(r_drawworld, 1, CVAR_CHEAT),
	CVAR_VAR(r_novis, 0, 0),
	CVAR_VAR(gl_frustumCull, 1, 0),
	CVAR_VAR(r_lerpmodels, 1, 0),

	CVAR_VAR(r_speeds, 0, 0),
	CVAR_VAR(r_ignorehwgamma, 0, CVAR_ARCHIVE),

	CVAR_VAR(gl_nosubimage, 0, 0),
	CVAR_VAR(gl_allow_software, 0, 0),

//	CVAR_VAR(gl_particle_min_size, 2, CVAR_ARCHIVE),
//	CVAR_VAR(gl_particle_max_size, 40, CVAR_ARCHIVE),
//	CVAR_VAR(gl_particle_size, 40, CVAR_ARCHIVE),
//	CVAR_VAR(gl_particle_att_a, 0.01, CVAR_ARCHIVE),
//	CVAR_VAR(gl_particle_att_b, 0, CVAR_ARCHIVE),
//	CVAR_VAR(gl_particle_att_c, 0.01, CVAR_ARCHIVE),

	CVAR_VAR(gl_modulate, 1, CVAR_ARCHIVE),
	CVAR_VAR(gl_logFile, 0, 0),
	CVAR_VAR(gl_bitdepth, 0, CVAR_ARCHIVE),
	CVAR_VAR(gl_mode, 3, CVAR_ARCHIVE),
	CVAR_VAR(gl_shadows, 0, CVAR_ARCHIVE ),
	CVAR_VAR(gl_dynamic, 1, 0),
	CVAR_VAR(gl_dlightBacks, 1, 0),
	CVAR_VAR(gl_nobind, 0, 0),
	CVAR_FULL(&gl_round_down, "gl_roundImagesDown", "0", CVAR_ARCHIVE),
	CVAR_VAR(gl_picmip, 0, 0),
	CVAR_VAR(gl_skymip, 0, 0),
	CVAR_VAR(gl_showtris, 0, CVAR_CHEAT),
	CVAR_VAR(gl_ztrick, 0, 0),
	CVAR_VAR(gl_finish, 0, CVAR_ARCHIVE),
	CVAR_VAR(gl_clear, 0, 0),
	CVAR_VAR(gl_cull, 1, 0),
	CVAR_VAR(gl_polyblend, 1, 0),
	CVAR_VAR(gl_flashblend, 0, 0),
	CVAR_VAR(gl_playermip, 0, 0),
	CVAR_VAR(gl_monolightmap, 0, 0),
	CVAR_VAR(gl_driver, opengl32, CVAR_ARCHIVE),
	CVAR_VAR(gl_texturemode, bilinear, CVAR_ARCHIVE),
	CVAR_VAR(gl_texturealphamode, default, CVAR_ARCHIVE),
	CVAR_VAR(gl_texturesolidmode, default, CVAR_ARCHIVE),
	CVAR_VAR(gl_lockpvs, 0, 0),
	CVAR_VAR(gl_zmax, 2300, CVAR_ARCHIVE),
	CVAR_VAR(gl_winrefl, 1, CVAR_ARCHIVE),

	CVAR_VAR(gl_vertex_arrays, 0, CVAR_ARCHIVE),

	CVAR_VAR(gl_swapinterval, 0, CVAR_ARCHIVE|CVAR_UPDATE),

	CVAR_VAR(gl_saturatelighting, 0, 0),

	CVAR_VAR(gl_3dlabs_broken, 1, CVAR_ARCHIVE),
CVAR_END

	Cvar_GetVars (ARRAY_ARG(vars));

	RegisterCommand ("imagelist", GL_ImageList_f);
	RegisterCommand ("modellist", Mod_Modellist_f);
	RegisterCommand ("gfxinfo", GL_Strings_f);
}

/*
==================
R_SetMode
==================
*/
bool R_SetMode (void)
{
	rserr_t err;
	bool	fullscreen;

	fullscreen = r_fullscreen->integer != 0;

	r_fullscreen->modified = false;
	gl_mode->modified = false;

	if ( ( err = GLimp_SetMode( &vid.width, &vid.height, gl_mode->integer, fullscreen ) ) == rserr_ok )
	{
		gl_config.prevMode = gl_mode->integer;
	}
	else
	{
		if ( err == rserr_invalid_fullscreen )
		{
			Cvar_SetInteger ("r_fullscreen", 0);
			r_fullscreen->modified = false;
			Com_Printf ("ref_gl::R_SetMode() - fullscreen unavailable in this mode\n");
			if ( ( err = GLimp_SetMode( &vid.width, &vid.height, gl_mode->integer, false ) ) == rserr_ok )
				return true;
		}
		else if ( err == rserr_invalid_mode )
		{
			Cvar_SetInteger ("gl_mode", gl_config.prevMode);
			gl_mode->modified = false;
			Com_Printf ("ref_gl::R_SetMode() - invalid mode\n");
		}

		// try setting it back to something safe
		if ( ( err = GLimp_SetMode( &vid.width, &vid.height, gl_config.prevMode, false ) ) != rserr_ok )
		{
			Com_Printf ("ref_gl::R_SetMode() - could not revert to safe mode\n");
			return false;
		}
	}
	return true;
}

/*
===============
R_Init
===============
*/
int R_Init( void )
{
	char renderer_buffer[1000];
	char vendor_buffer[1000];
	int		err;
	int		j;
	extern float r_turbsin[256];

	for (j = 0; j < 256; j++)
	{
		r_turbsin[j] = 4 * sin (j / 128.0f * M_PI);		// 0 -- 0, 256 -- 2*pi
	}

	Com_Printf ("ref_gl version: "REF_VERSION"\n");

	Draw_GetPalette ();

	R_Register();

	// initialize our QGL dynamic bindings
	if ( !QGL_Init( gl_driver->string ) )
	{
		QGL_Shutdown();
        Com_Printf ("ref_gl::R_Init() - could not load \"%s\"\n", gl_driver->string );
		return -1;
	}

	// initialize OS-specific parts of OpenGL
	if ( !GLimp_Init() )
	{
		QGL_Shutdown();
		return -1;
	}

	// set our "safe" modes
	gl_config.prevMode = 3;

	// create the window and set up the context
	if ( !R_SetMode () )
	{
		QGL_Shutdown();
        Com_Printf ("ref_gl::R_Init() - could not R_SetMode()\n" );
		return -1;
	}

	/*
	** get our various GL strings
	*/
	strcpy (gl_config.vendorString, (char*)glGetString (GL_VENDOR));
	Com_Printf ("GL_VENDOR: %s\n", gl_config.vendorString );
	strcpy (gl_config.rendererString, (char*)glGetString (GL_RENDERER));
	Com_Printf ("GL_RENDERER: %s\n", gl_config.rendererString );
	strcpy (gl_config.versionString, (char*)glGetString (GL_VERSION));
	Com_Printf ("GL_VERSION: %s\n", gl_config.versionString );
//??	strcpy (gl_config.extensions_string, glGetString (GL_EXTENSIONS));
//??	Com_Printf ("GL_EXTENSIONS: %s\n", gl_config.extensions_string );

	strcpy (renderer_buffer, gl_config.rendererString);
	strlwr (renderer_buffer );

	strcpy (vendor_buffer, gl_config.vendorString);
	strlwr (vendor_buffer );

	if ( strstr( renderer_buffer, "voodoo" ) )
	{
		if ( !strstr( renderer_buffer, "rush" ) )
			gl_config.renderer = GL_RENDERER_VOODOO;
		else
			gl_config.renderer = GL_RENDERER_VOODOO_RUSH;
	}
	else if ( strstr( vendor_buffer, "sgi" ) )
		gl_config.renderer = GL_RENDERER_SGI;
	else if ( strstr( renderer_buffer, "permedia" ) )
		gl_config.renderer = GL_RENDERER_PERMEDIA2;
	else if ( strstr( renderer_buffer, "glint" ) )
		gl_config.renderer = GL_RENDERER_GLINT_MX;
	else if ( strstr( renderer_buffer, "glzicd" ) )
		gl_config.renderer = GL_RENDERER_REALIZM;
	else if ( strstr( renderer_buffer, "gdi" ) )
		gl_config.renderer = GL_RENDERER_MCD;
	else if ( strstr( renderer_buffer, "pcx2" ) )
		gl_config.renderer = GL_RENDERER_PCX2;
	else if ( strstr( renderer_buffer, "verite" ) )
		gl_config.renderer = GL_RENDERER_RENDITION;
	else
		gl_config.renderer = GL_RENDERER_OTHER;

	if (gl_monolightmap->string[1] != 'F' && gl_monolightmap->string[1] != 'f')
	{
		if ( gl_config.renderer == GL_RENDERER_PERMEDIA2 )
		{
			Cvar_Set( "gl_monolightmap", "A" );
			Com_Printf( "...using gl_monolightmap 'a'\n" );
		}
		else if ( gl_config.renderer & GL_RENDERER_POWERVR )
		{
			Cvar_Set( "gl_monolightmap", "0" );
		}
		else
		{
			Cvar_Set( "gl_monolightmap", "0" );
		}
	}

	// power vr can't have anything stay in the framebuffer, so
	// the screen needs to redraw the tiled background every frame
	if ( gl_config.renderer & GL_RENDERER_POWERVR )
	{
		Cvar_Set( "scr_drawall", "1" );
	}
	else
	{
		Cvar_Set( "scr_drawall", "0" );
	}

#ifdef __linux__
	Cvar_SetInteger ("gl_finish", 1);
#endif

	// MCD has buffering issues
	if ( gl_config.renderer == GL_RENDERER_MCD )
	{
		Cvar_SetInteger ("gl_finish", 1);
	}

	if ( gl_config.renderer & GL_RENDERER_3DLABS )
	{
		if ( gl_3dlabs_broken->integer )
			gl_config.allow_cds = false;
		else
			gl_config.allow_cds = true;
	}
	else
	{
		gl_config.allow_cds = true;
	}

	if ( gl_config.allow_cds )
		Com_Printf( "...allowing CDS\n" );
	else
		Com_Printf( "...disabling CDS\n" );

	/*------------ grab extensions ---------------*/

	QGL_InitExtensions ();

/*	if ( strstr( gl_config.extensions_string, "GL_EXT_point_parameters" ) )
	{
		if ( gl_ext_pointparameters->integer )
		{
			glPointParameterfEXT = ( void (APIENTRY *)( GLenum, GLfloat ) ) qwglGetProcAddress( "glPointParameterfEXT" );
			glPointParameterfvEXT = ( void (APIENTRY *)( GLenum, const GLfloat * ) ) qwglGetProcAddress( "glPointParameterfvEXT" );
			Com_Printf( "...using GL_EXT_point_parameters\n" );
		}
		else
		{
			Com_Printf( "...ignoring GL_EXT_point_parameters\n" );
		}
	}
	else
	{
		Com_Printf( "...GL_EXT_point_parameters not found\n" );
	}
*/
#ifdef __linux__
	if ( strstr( gl_config.extensions_string, "3DFX_set_global_palette" ))
	{
		if ( gl_ext_palettedtexture->integer )
		{
			Com_Printf( "...using 3DFX_set_global_palette\n" );
			qgl3DfxSetPaletteEXT = ( void ( APIENTRY * ) (GLuint *) )qwglGetProcAddress( "gl3DfxSetPaletteEXT" );
			glColorTableEXT = Fake_glColorTableEXT;
		}
		else
		{
			Com_Printf( "...ignoring 3DFX_set_global_palette\n" );
		}
	}
	else
	{
		Com_Printf( "...3DFX_set_global_palette not found\n" );
	}
#endif

/*	if ( !glColorTableEXT &&
		strstr( gl_config.extensions_string, "GL_EXT_paletted_texture" ) &&
		strstr( gl_config.extensions_string, "GL_EXT_shared_texture_palette" ) )
	{
		if ( gl_ext_palettedtexture->integer )
		{
			Com_Printf( "...using GL_EXT_shared_texture_palette\n" );
			glColorTableEXT = ( void ( APIENTRY * ) ( int, int, int, int, int, const void * ) ) qwglGetProcAddress( "glColorTableEXT" );
		}
		else
		{
			Com_Printf( "...ignoring GL_EXT_shared_texture_palette\n" );
		}
	}
	else
	{
		Com_Printf( "...GL_EXT_shared_texture_palette not found\n" );
	}
*/

/*	if (GL_SUPPORT(QGL_ARB_MULTITEXTURE))
	{
		glGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &gl_config.maxActiveTextures);
//			Com_Printf ("...using GL_ARB_multitexture with %d texture units\n", gl_config.maxActiveTextures);
	}
	else if (GL_SUPPORT(QGL_SGIS_MULTITEXTURE))
	{
		glGetIntegerv (GL_MAX_TEXTURES_SGIS, &gl_config.maxActiveTextures);
	}
	else
		gl_config.maxActiveTextures = 1;
*/
	gl_state.texture_format_solid = 0;
	gl_state.texture_format_alpha = 0;
	gl_state.texture_format_alpha1 = 0;

	GL_SetDefaultState();

#if 0 // commented out until H3D pays us the money they owe us
	/*
	** draw our stereo patterns
	*/
	GL_DrawStereoPattern();
#endif

	GL_InitImages ();
	Mod_Init ();
	R_InitParticleTexture ();
	Draw_InitLocal ();

	err = glGetError();
	if (err != GL_NO_ERROR)
		Com_Printf ("glGetError() = 0x%x\n", err);
	return 0;
}

/*
===============
R_Shutdown
===============
*/
void R_Shutdown (void)
{
	UnregisterCommand ("modellist");
	UnregisterCommand ("imagelist");
	UnregisterCommand ("gfxinfo");

	Mod_FreeAll ();

	GL_ShutdownImages ();

	/*
	** shut down OS specific OpenGL stuff like contexts, etc.
	*/
	GLimp_Shutdown();

	/*
	** shutdown our QGL subsystem
	*/
	QGL_Shutdown();
}



/*
@@@@@@@@@@@@@@@@@@@@@
R_BeginFrame
@@@@@@@@@@@@@@@@@@@@@
*/
void R_BeginFrame( float camera_separation )
{
	if (gl_logFile->modified)
	{
		QGL_EnableLogging (gl_logFile->integer != 0);
		gl_logFile->modified = false;
	}
	else if (gl_logFile->integer == 2)
	{
		QGL_EnableLogging (false);
		Cvar_SetInteger ("gl_logFile", 0);
	}

	gl_state.camera_separation = camera_separation;

	gl_speeds.beginFrame = Sys_Milliseconds ();
	gl_speeds.numBinds = gl_speeds.numUploads = 0;
	/*
	** change modes if necessary
	*/
	if ( gl_mode->modified || r_fullscreen->modified )
	{	// FIXME: only restart if CDS is required
		vid_ref->modified = true;
	}

	LOG_STRING ("\n---------- Begin Frame (old ref) ----------\n\n");

	/*
	** update hardware gamma (if present)
	*/
	if ((r_gamma->modified || r_contrast->modified || r_brightness->modified) && GLimp_HasGamma ())
	{
//		intensity->modified = false;
		GLimp_SetGamma (r_gamma->value, 0 /* intensity->value */);
		r_gamma->modified = false;
		r_contrast->modified = false;
		r_brightness->modified = false;
	}

	/*
	** update 3Dfx gamma -- it is expected that a user will do a vid_restart
	** after tweaking this value
	*/
	if ( r_gamma->modified )
	{
		r_gamma->modified = false;

		if ( gl_config.renderer & ( GL_RENDERER_VOODOO ) )
		{
			float g = 2.00 * ( 0.8 - ( r_gamma->value - 0.5 ) ) + 1.0F;
			putenv(va("SSTV2_GAMMA=%f", g));
			putenv(va("SST_GAMMA=%f", g));
		}
	}

	GLimp_BeginFrame( camera_separation );

	/*
	** go into 2D mode
	*/
	glViewport (0,0, vid.width, vid.height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	glOrtho  (0, vid.width, vid.height, 0, -BIG_NUMBER, BIG_NUMBER);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity ();
	glDisable (GL_DEPTH_TEST);
	glDisable (GL_CULL_FACE);
	glDisable (GL_BLEND);
	glEnable (GL_ALPHA_TEST);
	glColor4f (1,1,1,1);

	/*
	** texturemode stuff
	*/
	if ( gl_texturemode->modified )
	{
		GL_TextureMode( gl_texturemode->string );
		gl_texturemode->modified = false;
	}

	if ( gl_texturealphamode->modified )
	{
		GL_TextureAlphaMode( gl_texturealphamode->string );
		gl_texturealphamode->modified = false;
	}

	if ( gl_texturesolidmode->modified )
	{
		GL_TextureSolidMode( gl_texturesolidmode->string );
		gl_texturesolidmode->modified = false;
	}

	/*
	** swapinterval stuff
	*/
	GL_UpdateSwapInterval();

	//
	// clear screen if desired
	//
	R_Clear ();
}

static void R_EndFrame (void)
{
	if (gl_screenshotName)
		GL_PerformScreenshot ();

	GLimp_EndFrame ();

	if (gl_finish->integer)	// old place: beginning of the R_RenderFrame()
		glFinish ();

	gl_speeds.endFrame = Sys_Milliseconds ();
	if (r_speeds->integer)
	{
		DrawTextRight (va("bef: %2d fe: %2d bk: %2d aft: %2d",
			gl_speeds.beginWorld - gl_speeds.beginFrame,
			gl_speeds.beginBk - gl_speeds.beginWorld,
			gl_speeds.endBk - gl_speeds.beginBk,
			gl_speeds.endFrame - gl_speeds.endBk
			), RGB(1, 0.5, 0));
		DrawTextRight (va("binds: %2d uploads: %2d",
			gl_speeds.numBinds, gl_speeds.numUploads
			), RGB(1, 0.5, 0));
	}
}

/*
=============
R_SetPalette
=============
*/
unsigned r_rawpalette[256];

void R_SetPalette ( const byte *palette)
{
	int		i;

	byte *rp = ( byte * ) r_rawpalette;

	if ( palette )
	{
		for ( i = 0; i < 256; i++ )
		{
			rp[i*4+0] = palette[i*3+0];
			rp[i*4+1] = palette[i*3+1];
			rp[i*4+2] = palette[i*3+2];
			rp[i*4+3] = 0xff;
		}
	}
	else
	{
		for ( i = 0; i < 256; i++ )
		{
			rp[i*4+0] = d_8to24table[i] & 0xff;
			rp[i*4+1] = ( d_8to24table[i] >> 8 ) & 0xff;
			rp[i*4+2] = ( d_8to24table[i] >> 16 ) & 0xff;
			rp[i*4+3] = 0xff;
		}
	}
	GL_SetTexturePalette( r_rawpalette );

	glClearColor (0,0,0,0);
	glClear (GL_COLOR_BUFFER_BIT);
	glClearColor (1,0, 0.5 , 0.5);
}

/*
** R_DrawBeam
*/
void R_DrawBeam (beam_t *e)
{
#define NUM_BEAM_SEGS 6

	int	i;
	float r, g, b;

	vec3_t perpvec;
	vec3_t direction, normalized_direction;
	vec3_t	start_points[NUM_BEAM_SEGS], end_points[NUM_BEAM_SEGS];
	vec3_t oldorigin, origin;

	oldorigin[0] = e->start[0];
	oldorigin[1] = e->start[1];
	oldorigin[2] = e->start[2];

	origin[0] = e->end[0];
	origin[1] = e->end[1];
	origin[2] = e->end[2];

	normalized_direction[0] = direction[0] = oldorigin[0] - origin[0];
	normalized_direction[1] = direction[1] = oldorigin[1] - origin[1];
	normalized_direction[2] = direction[2] = oldorigin[2] - origin[2];

	if ( VectorNormalize( normalized_direction ) == 0 )
		return;

	PerpendicularVector( perpvec, normalized_direction );
	VectorScale( perpvec, e->radius, perpvec );

	for ( i = 0; i < 6; i++ )
	{
		RotatePointAroundVector( start_points[i], normalized_direction, perpvec, (360.0/NUM_BEAM_SEGS)*i );
		VectorAdd( start_points[i], origin, start_points[i] );
		VectorAdd( start_points[i], direction, end_points[i] );
	}

	glDisable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glDepthMask( GL_FALSE );

	r = d_8to24table[e->color.c[0] & 0xFF] & 0xFF;
	g = (d_8to24table[e->color.c[0] & 0xFF] >> 8) & 0xFF;
	b = (d_8to24table[e->color.c[0] & 0xFF] >> 16) & 0xFF;

	glColor4f( r / 255.0f, g / 255.0f, b / 255.0f, e->alpha );

	glBegin( GL_TRIANGLE_STRIP );
	for ( i = 0; i < NUM_BEAM_SEGS; i++ )
	{
		glVertex3fv( start_points[i] );
		glVertex3fv( end_points[i] );
		glVertex3fv( start_points[(i+1)%NUM_BEAM_SEGS] );
		glVertex3fv( end_points[(i+1)%NUM_BEAM_SEGS] );
	}
	glEnd();

	glEnable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );
}

//===================================================================


void	R_BeginRegistration (const char *map);
struct model_s	*R_RegisterModel (const char *name);
struct image_s	*R_RegisterSkin (const char *name);
void R_SetSky (const char *name, float rotate, vec3_t axis);
void	R_EndRegistration (void);

void	R_RenderFrame (refdef_t *fd);

struct image_s	*Draw_FindPic (const char *name);

/*
Draw_ConCharColor
*/
void	Draw_ConCharColor (int x, int y, int c, int color)
{
	Draw_CharColor (x * 8, y * 8, c, color);
}

static void Screenshot (int flags, const char *name)
{
	static char shotName[MAX_QPATH];

	strcpy (shotName, name);
	gl_screenshotFlags = flags;
	gl_screenshotName = shotName;
}

static float GetClientLight (void)
{
	return gl_lightlevel;
}

static void ReloadImage (const char *name)
{
	// do nothing in this renderer ??
}

/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI

@@@@@@@@@@@@@@@@@@@@@
*/
refExport_t GetRefAPI (const refImport_t *rimp)
{
	refExport_t	re;

#ifdef DYNAMIC_REF
	ri = *rimp;
	if (ri.struc_size != sizeof(refImport_t))
	{
		re.struc_size = 0;
		return re;
	}
	InitRendererVars ();
#endif

	re.struc_size = sizeof(re);
	re.flags = &ref_flags;
	ref_flags = REF_USE_PALETTE;
	if (Cvar_Get ("gl_console_only", "0", 0)->integer)
		ref_flags |= REF_CONSOLE_ONLY;

	re.Init = R_Init;
	re.Shutdown = R_Shutdown;
	re.BeginFrame = R_BeginFrame;
	re.EndFrame = R_EndFrame;
	re.AppActivate = GLimp_AppActivate;
	re.DrawConChar = Draw_ConCharColor;
	re.Screenshot = Screenshot;

	re.RenderFrame = R_RenderFrame;
	re.BeginRegistration = R_BeginRegistration;
	re.RegisterModel = R_RegisterModel;
	re.RegisterSkin = R_RegisterSkin;
	re.RegisterPic = Draw_FindPic;
	re.SetSky = R_SetSky;
	re.EndRegistration = R_EndRegistration;

	re.ReloadImage = ReloadImage;
	re.DrawGetPicSize = Draw_GetPicSize;
	re.DrawPic = Draw_PicColor;
	re.DrawStretchPic = Draw_StretchPic;
	re.DrawDetailedPic = Draw_StretchPic;
	re.DrawChar = Draw_CharColor;
	re.DrawTileClear = Draw_TileClear;
	re.DrawFill = Draw_Fill;
	re.DrawFill2 = Draw_Fill2;

	re.DrawStretchRaw8 = Draw_StretchRaw8;
	re.SetRawPalette = R_SetPalette;

	re.DrawTextPos = DrawTextPos;
	re.DrawTextLeft = DrawTextLeft;
	re.DrawTextRight = DrawTextRight;

	re.GetClientLight = GetClientLight;

	Swap_Init ();

	return re;
}