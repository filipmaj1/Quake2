#include "gl_local.h"
#include "gl_shader.h"


shader_t *gl_defaultShader;
shader_t *gl_identityLightShader;
shader_t *gl_concharsShader;
shader_t *gl_defaultSkyShader;		// default sky shader (black image)
shader_t *gl_skyShader;				// current sky shader (have mapped images)
shader_t *gl_alphaShader1, *gl_alphaShader2;


#define HASH_BITS		8
#define HASH_SIZE		(1 << HASH_BITS)
#define HASH_MASK		(HASH_SIZE - 1)

#define	MAX_SHADERS		1024

/* Allocate all shaders dynamically. Reallocate every time new map loaded (do now perform partial cleanup) - this
   will be fast enough, if we will hold shader scripts in memory.
   Add refCount to image_t ??
 */
static shader_t *shadersArray[MAX_SHADERS];	// sorted in ascending order with key = sortParam
				// it's easier to keep array sorted, than sort it with "sortParam" later

static shader_t *hashTable[HASH_SIZE];
static int	shaderCount;

static void *shaderChain;


// name should be in a lower case
static int ComputeHash (char *name)
{
	int		h;
	char	*s, c;

	h = 0;
	s = name;
	while (c = *s++)
		h = (h ^ 0x25) + c;
	return h & HASH_MASK;
}


/*------- Initialization/finalization ----------*/

static void Shaderlist_f (void)
{
	int		i, n;
	static char *shTypes[] = {"", "sky", "fog", "por"};
	static char *boolNames[] = {" ", "+"};
	static char *badNames[] = {"", " ^1(errors)^7"};
	char	*mask;

	if (Cmd_Argc () > 2)
	{
		Com_Printf ("Usage: shaderlist [<mask>]\n");
		return;
	}

	if (Cmd_Argc () == 2)
		mask = Cmd_Argv (1);
	else
		mask = NULL;

	n = 0;
	Com_Printf ("----ns-f-lm-s--type-name--------\n");
	for (i = 0; i < shaderCount; i++)
	{
		shader_t *sh;
		char	lmInfo[16];

		sh = shadersArray[i];
		if (mask && !MatchWildcard (sh->name, mask)) continue;
		n++;

		lmInfo[1] = 0;
		switch (sh->lightmapNumber)
		{
		case LIGHTMAP_NONE:
			lmInfo[0] = ' ';
			break;
		case LIGHTMAP_VERTEX:
			lmInfo[0] = 'v';
			break;
		case LIGHTMAP_RESERVE:
			lmInfo[0] = 'r';
			break;
		default:
			Com_sprintf (lmInfo, sizeof(lmInfo), "%d", sh->lightmapNumber);
		}
		Com_Printf ("%-3d %d  %s %2s %-2g %3s  %s%s\n", i, sh->numStages, boolNames[sh->fast], lmInfo,
			sh->sortParam, shTypes[sh->type], sh->name, badNames[sh->bad]);
	}
	Com_Printf ("  %d shaders\n", n);
}


void GL_InitShaders (void)
{
	Cmd_AddCommand ("shaderlist", Shaderlist_f);

	/*------- reading scripts --------*/
	//!!

	GL_ResetShaders ();
}


void GL_ShutdownShaders (void)
{
	if (shaderChain)
	{
		FreeMemoryChain (shaderChain);
		shaderChain = NULL;
	}
	shaderCount = 0;
	Cmd_RemoveCommand ("shaderlist");
}


void GL_ResetShaders (void)
{
	if (shaderChain)
		FreeMemoryChain (shaderChain);
	shaderChain = CreateMemoryChain ();

	memset (hashTable, 0, sizeof(hashTable));
	memset (shadersArray, 0, sizeof(shadersArray));
	shaderCount = 0;

	/*--- creating system shaders ----*/
	// abstract shaders should be created in reverse (with relation to sortParam) order
	gl_alphaShader2 = GL_FindShader ("*alpha2", SHADER_ABSTRACT);
	gl_alphaShader2->sortParam = SORT_SEETHROUGH + 1;
	gl_alphaShader1 = GL_FindShader ("*alpha1", SHADER_ABSTRACT);
	gl_alphaShader1->sortParam = SORT_SEETHROUGH;

	gl_defaultShader = GL_FindShader ("*default", SHADER_WALL);
	gl_identityLightShader = GL_FindShader ("*identityLight", SHADER_FORCEALPHA);
	gl_skyShader = gl_defaultSkyShader = GL_FindShader ("*sky", SHADER_SKY|SHADER_ABSTRACT);

	gl_concharsShader = GL_FindShader ("pics/conchars", SHADER_ALPHA);
}


/*------------- Creating new shader ------------*/


static shader_t sh;
static shaderStage_t st[MAX_SHADER_STAGES];
static tcModParms_t tcMods[MAX_STAGE_TCMODS * MAX_SHADER_STAGES];


static void ResortShader (shader_t *shader, int startIndex)
{
	float	sort;
	int		i, j, numTextures, sort2;

	// compute secondary sort key
	numTextures = 0;
	sort2 = 0;
	for (i = 0; i < shader->numStages; i++)		// iterate stages
	{
		stageTMU_t *tmu;

		for (j = 0, tmu = shader->stages[i].tmu; j < 2; j++, tmu++)		// iterate tmu's
			if (shader->stages[i].tmu[j].numAnimTextures)
			{
				image_t *image;
				int		texnum;

				image = tmu->mapImage[0];

				if (image && !(shader->lightmapNumber == LIGHTMAP_RESERVE && tmu->isLightmap))
					texnum = image->texnum;
				else	// ignore image if this is reserved lightmap
					texnum = 1023;		// large positive number (put it to the end of sortParam2, so ResortShader() will work later)
				sort2 = (sort2 << 10) | texnum;

				if (++numTextures == 3)	// not enough place in sortParam2
					break;
			}
	}
	shader->sortParam2 = sort2;

	// insert into a shadersArray (sorted)
	sort = shader->sortParam;
	for (i = startIndex - 1; i >= 0; i--)
	{
		shader_t *ash;	// array shader

		ash = shadersArray[i];
		if (sort < ash->sortParam || (sort == ash->sortParam && sort2 < ash->sortParam2))
		{
			// move this shader down
			ash->sortIndex++;
			shadersArray[i + 1] = ash;
		}
		else
		{
			// insert new shader after this place
			i++;
			break;
		}
	}
	if (i != startIndex && i < gl_state.minNewShaderIndex)
		gl_state.minNewShaderIndex = i;
	if (i < 0) i = 0;	// will be first (for example, when shaderCount == 0)
	shadersArray[i] = shader;
	shader->sortIndex = i;
}


// Insert shader to shaders array
static shader_t *CreatePermanentShader (void)
{
	shader_t 		*nsh;	// new shader
	shaderStage_t	*nst;	// new stages
	int				size, hash;

	if (shaderCount >= MAX_SHADERS)
	{
		Com_WPrintf ("CreatePermanentShader(%s): MAX_SHADERS hit\n", sh.name);
		return gl_defaultShader;
	}

	// we may change shader sort indexes, so - flush scene
//	GL_BackEnd (); -- frame will be dropped??

	// allocate and copy new shader
	nsh = AllocChainBlock (shaderChain, sizeof(shader_t));
	size = sh.numStages * sizeof(shaderStage_t);
	nst = AllocChainBlock (shaderChain, size);
	sh.stages = nst;
	memcpy (nsh, &sh, sizeof(sh));
	memcpy (nst, &st[0], size);

	// insert into a hash table
	hash = ComputeHash (sh.name);
	nsh->hashNext = hashTable[hash];
	hashTable[hash] = nsh;

	ResortShader (nsh, shaderCount);
	shaderCount++;

	return nsh;
}


static shader_t *FinishShader (void)
{
	int		numStages;

	if (sh.type == SHADERTYPE_SKY)
		sh.sortParam = SORT_SKY;
	else if (!sh.sortParam && sh.usePolygonOffset)
		sh.sortParam = SORT_DECAL;

	if (!gl_ignoreFastPath->integer) sh.fast = true;
	if (sh.numDeforms || sh.usePolygonOffset) sh.fast = false;

	// enum and count stages
	for (numStages = 0; numStages < MAX_SHADER_STAGES; numStages++)
	{
		shaderStage_t *s;

		s = &st[numStages];
		if (!s->tmu[0].numAnimTextures) break;

		// determine tcGenType
		if (s->tmu[0].isLightmap)
		{	// lightmap stage
			if (!s->tmu[0].tcGenType)
				s->tmu[0].tcGenType = TCGEN_LIGHTMAP;
		}
		else
		{	// texture stage
			if (!s->tmu[0].tcGenType)
				s->tmu[0].tcGenType = TCGEN_TEXTURE;
		}

		// process rgbGen
		if (s->rgbGenType == RGBGEN_VERTEX && gl_config.identityLightValue_f == 1.0f)
			s->rgbGenType = RGBGEN_EXACT_VERTEX;	// faster to fill
		switch (s->rgbGenType)
		{
		case RGBGEN_NONE:	// rgbGen is not set
		case RGBGEN_IDENTITY:
			s->rgbGenConst[0] = s->rgbGenConst[1] = s->rgbGenConst[2] = 1;
			s->rgbGenType = RGBGEN_CONST;
			break;
		case RGBGEN_IDENTITY_LIGHTING:
			s->rgbGenConst[0] = s->rgbGenConst[1] = s->rgbGenConst[2] = gl_config.identityLightValue_f;
			s->rgbGenType = RGBGEN_CONST;
			break;
		}

		// process alphaGen
		if (s->alphaGenType == ALPHAGEN_IDENTITY)
		{
			s->alphaGenConst = 1;
			s->alphaGenType = ALPHAGEN_CONST;
		}

		// process blend mode
		if (s->glState & (GLSTATE_SRCMASK|GLSTATE_DSTMASK) &&
			st[0].glState & (GLSTATE_SRCMASK|GLSTATE_DSTMASK))
		{	// have blending
/*--- determine fog type depends on blend mode ---
 if (src == ZERO && DST= = ONE_MINUS_SRC_COLOR)
     fogType = RGB;
 else if (src == SRC_ALPHA && dst == ONE_MINUS_SRC_ALPHA)
     fogType = ALPHA;
 else if (dst == ONE || dst == ONE_MINUS_SRC_ALPHA)
     fogType = RGBA;
 */
			if (!sh.sortParam)
			{
				if (s->glState & GLSTATE_DEPTHWRITE)
					sh.sortParam = SORT_SEETHROUGH;
				else
					sh.sortParam = SORT_SPRITE;
			}
		}

		// process tcMod
		if (s->tmu[0].numTcMods)
		{
			tcModParms_t *tc;
			int		size;

			size = s->tmu[0].numTcMods * sizeof(tcModParms_t);
			tc = AllocChainBlock (shaderChain, size);
			memcpy (tc, s->tmu[0].tcModParms, size);
			s->tmu[0].tcModParms = tc;
		}

		// check for fast draw ability
		if (!((s->rgbGenType == RGBGEN_CONST && s->alphaGenType == ALPHAGEN_CONST) ||			// rgba-gen const
			  (s->rgbGenType == RGBGEN_EXACT_VERTEX && s->alphaGenType == ALPHAGEN_VERTEX)) ||	// rgba-gen exact vertex
			s->tmu[0].numTcMods || (s->tmu[0].tcGenType != TCGEN_TEXTURE && s->tmu[0].tcGenType != TCGEN_LIGHTMAP))	// tcgen/tcmod
			sh.fast = false;
	}
	sh.numStages = numStages;

	if (!sh.sortParam)
		sh.sortParam = SORT_OPAQUE;

	// optimize stages (vertex lighting (simplify ?), multitexturing ...)

	return CreatePermanentShader ();
}


image_t *GetLightmapImage (int num)
{
	if (num == LIGHTMAP_RESERVE)
		return NULL;

	return GL_FindImage (va("*lightmap%d", num), IMAGE_CLAMP);
}


shader_t *GL_SetShaderLightmap (shader_t *shader, int lightmapNumber)
{
	int			hash, i;
	shader_t	*s, *dest;

	if (shader->lightmapNumber == LIGHTMAP_VERTEX)
		return shader;

	hash = ComputeHash (shader->name);
	dest = NULL;
	for (s = hashTable[hash]; s; s = s->hashNext)
		if (!strcmp (shader->name, s->name) && shader->style == s->style)
		{
			if (s->lightmapNumber == lightmapNumber)
				return s;	// found exact shader as required

			if (s->lightmapNumber == LIGHTMAP_RESERVE)
			{	// found shader with reserved lightmap
				dest = s;
				break;
			}
		}

	if (!dest)
	{
		// shader not found -- duplicate source and change its lightmap
		memcpy (&sh, shader, sizeof(shader_t));
		memcpy (&st[0], shader->stages, shader->numStages * sizeof(shaderStage_t));
		sh.lightmapNumber = LIGHTMAP_RESERVE;	// temporarily mark as reserved
		dest = CreatePermanentShader ();
		// new shader's tcMod will point to the old shader's tcMod stages
	}
	dest->lightmapNumber = lightmapNumber;

	// find lightmap stage / TMU
	for (i = 0; i < dest->numStages; i++)
	{
		int		j;
		stageTMU_t *tmu;

		for (j = 0, tmu = dest->stages[i].tmu; j < 2 && tmu->numAnimTextures; j++, tmu++)
			if (tmu->isLightmap)
			{
				tmu->mapImage[0] = GetLightmapImage (lightmapNumber);
				ResortShader (dest, dest->sortIndex);
				return dest;
			}
	}

	Com_Error (ERR_DROP, "R_SetShaderLightmap(%s, %d): lightmap stage is not found", shader->name, lightmapNumber);
	return NULL;	// make compiler happy (will not go here)
}


shader_t *GL_GetAlphaShader (shader_t *shader)
{
	if (shader->alphaShader)
		return shader->alphaShader;		// already done

	if (shader->stages[0].glState & (GLSTATE_SRCMASK|GLSTATE_DSTMASK))
	{	// already have blend mode
		shader->alphaShader = shader;
		Com_DPrintf ("R_GetAlphaShader(%s): already have blend\n", shader->name);
		return shader;
	}

	// clone shader
	memcpy (&sh, shader, sizeof(shader_t));
	memcpy (&st[0], shader->stages, shader->numStages * sizeof(shaderStage_t));
	strcat (sh.name, "#a");				// just make different name
	// setup alpha/blend
	sh.sortParam = SORT_SEETHROUGH;		//?? SORT_SPRITE? check SHADER_WALL ??
	st[0].alphaGenType = ALPHAGEN_ENTITY;
	st[0].glState |= GLSTATE_SRC_SRCALPHA|GLSTATE_DST_ONEMINUSSRCALPHA;
	st[0].glState &= ~GLSTATE_DEPTHWRITE;

	shader->alphaShader = FinishShader ();
	return shader->alphaShader;
}


static int numTcModStages;

static tcModParms_t *NewTcModStage (shaderStage_t *stage)
{
	tcModParms_t *par;

	if (stage->tmu[0].numTcMods > MAX_STAGE_TCMODS)
		Com_Error (ERR_DROP, "Too many tcMod stages in shader \"%s\"\n", sh.name);
	//?? check index overflow (no MAX_STAGE_TCMODS but MAX_SHADER_TCMODS ??)
	par = &tcMods[numTcModStages++];	// alloc
	if (!stage->tmu[0].numTcMods)
		stage->tmu[0].tcModParms = par;		// if 1st tcMod in stage -- setup pointer
	stage->tmu[0].numTcMods++;
	return par;
}


// "mipmap" is used only for auto-generated shaders
shader_t *GL_FindShader (char *name, int style)
{
	char	name2[MAX_QPATH], *s;
	int		hash, lightmapNumber, imgFlags;
	shader_t *shader;

	if (style & SHADER_WALL)
		imgFlags = IMAGE_PICMIP|IMAGE_MIPMAP;
	else
		imgFlags = 0;

	if (style & SHADER_LIGHTMAP && gl_singleShader->integer)
	{
		style &= ~SHADER_ANIM;	// no animation in this mode
		name = "*default";		// use this image
	}

	Q_CopyFilename (name2, name, sizeof(name2)-1);
	if (s = strrchr (name2, '.')) *s = 0;		// cut extension

	if (style & SHADER_LIGHTMAP)
	{
		if (gl_config.vertexLight)
			lightmapNumber = LIGHTMAP_VERTEX;
		else
			lightmapNumber = LIGHTMAP_RESERVE;
	}
	else
		lightmapNumber = LIGHTMAP_NONE;

	/*----- find shader using hash table -----*/
	hash = ComputeHash (name2);
	for (shader = hashTable[hash]; shader; shader = shader->hashNext)
		if (!strcmp (shader->name, name2))
		{
			if (shader->bad)					// generated by bad script
				return shader;

			if (shader->style == (style & SHADER_STYLEMASK) || shader->scripted)
			{
				if (shader->lightmapNumber == lightmapNumber)
					return shader;				// exact as required

				if (lightmapNumber == LIGHTMAP_RESERVE)
					return shader;				// want to get reserved -- return any

				if (shader->lightmapNumber == LIGHTMAP_RESERVE)
					return GL_SetShaderLightmap (shader, lightmapNumber);	// reserved -> used
			}
		}

	// prepate common shader fields
	memset (&sh, 0, sizeof(sh));
	memset (&st, 0, sizeof(st));
	memset (&tcMods, 0, sizeof(tcMods));
	numTcModStages = 0;
	strcpy (sh.name, name2);
	sh.lightmapNumber = lightmapNumber;

//!!	if ()
	/*------------ find script ---------------*/
//	{	WARNING: fill shader->width and height with params of the main image
//	}
//	else
	/*----- create shader without script -----*/
	{
		image_t *img;

		sh.style = style & SHADER_STYLEMASK;

		if (style & SHADER_SKY)
		{
			int		i;

			sh.type = SHADERTYPE_SKY;

			if (style & SHADER_ABSTRACT)
				return FinishShader ();

			for (i = 0; i < 6; i++)
			{
				static char *suff[6] = {"rt", "lf", "bk", "ft", "up", "dn"};

				img = GL_FindImage (va("%s%s", name2, suff[i]), IMAGE_CLAMP);
				if (!img)
				{
					sh.bad = true;
					break;
				}
				sh.skyFarBox[i] = img;
			}
			if (!sh.bad)							// valid sky
				return FinishShader ();
			else
				return gl_defaultSkyShader;
		}

		// regular shader
		st[0].tmu[0].numAnimTextures = 1;

		if (style & SHADER_ABSTRACT)
			img = gl_defaultImage;					// just any image (will be removed later anyway)
		else
			img = GL_FindImage (name, imgFlags);	// use "name", not "name2" (full name, may be - with extension)

		if (!img)
		{
			if (style & SHADER_CHECK)
				return NULL;	// do not return default shader

			Com_WPrintf ("R_FindShader: couldn't find image \"%s\"\n", name);
			sh.bad = true;
			img = gl_defaultImage;
		}
//		else -- if we enable this, map will not be loaded when wall texture absent (no lightmap stage will be created)
		{
			shaderStage_t *stage;

			sh.width = img->width;
			sh.height = img->height;

			stage = &st[0];

			if (style & SHADER_TRYLIGHTMAP)
			{
				//?? vertex lighting for alpha/warp-surfaces
				lightmapNumber = LIGHTMAP_NONE;
				stage->rgbGenType = RGBGEN_EXACT_VERTEX;
				sh.lightmapNumber = LIGHTMAP_VERTEX;
			}

			if (lightmapNumber >= 0)
			{
				stage->tmu[0].mapImage[0] = GetLightmapImage (lightmapNumber);
				stage->tmu[0].isLightmap = true;
				stage->glState = GLSTATE_DEPTHWRITE;
				stage->rgbGenType = RGBGEN_IDENTITY;
				stage->alphaGenType = ALPHAGEN_IDENTITY;
				stage++;	// add next stage
				stage->tmu[0].numAnimTextures = 1;
			}

			stage->tmu[0].mapImage[0] = img;
			if (style & SHADER_ANIM)
			{
				int		i;
				char	*pname;

				pname = strchr (name, 0) + 1;
				stage->tmu[0].animMapFreq = 2;		// standard Quake2 animation frequency
				for (i = 1; *pname && i < MAX_STAGE_TEXTURES; i++, pname = strchr(pname, 0)+1)
				{
					img = GL_FindImage (pname, imgFlags);
					if (!img)
					{
						Com_WPrintf ("R_FindShader: couldn't find image \"%s\"\n", pname);
						stage->tmu[0].numAnimTextures = 1;
						sh.bad = true;
						break;
					}
					stage->tmu[0].mapImage[i] = img;
					stage->tmu[0].numAnimTextures++;
				}
			}

			/*--------- processing style -------------*/
			if (lightmapNumber >= 0)
			{
//				stage->glState = GLSTATE_SRC_DSTCOLOR|GLSTATE_DST_ZERO;	// 2nd stage -- texture
				stage->glState = GLSTATE_SRC_DSTCOLOR|GLSTATE_DST_SRCCOLOR;	// 2nd stage -- texture
				stage->rgbGenType = RGBGEN_IDENTITY;
				stage->alphaGenType = ALPHAGEN_IDENTITY;
			}
			else if (lightmapNumber == LIGHTMAP_VERTEX)
			{
				stage->glState = GLSTATE_DEPTHWRITE;
				stage->rgbGenType = RGBGEN_EXACT_VERTEX;
				stage->alphaGenType = ALPHAGEN_VERTEX;	// no alpha for lightmapped surface, but rgb=vertex and alpha=vertex -- fast
			}
			else if (style & SHADER_FORCEALPHA)
			{
				stage->glState = GLSTATE_SRC_SRCALPHA|GLSTATE_DST_ONEMINUSSRCALPHA;
						// image has no alpha, but use glColor(x,x,x,<1)
				stage->alphaGenType = ALPHAGEN_VERTEX;
			}
			else if (style & SHADER_ALPHA && img->alphaType)
			{
				if (img->alphaType == 1)
					stage->glState = GLSTATE_ALPHA_GE05;
				else
					stage->glState = GLSTATE_SRC_SRCALPHA|GLSTATE_DST_ONEMINUSSRCALPHA|GLSTATE_DEPTHWRITE
						| GLSTATE_ALPHA_GT0;	//?? need this for ability to look through alpha-models to alpha-world
			}
			else if (style & (SHADER_TRANS33|SHADER_TRANS66))
			{
				stage->glState = GLSTATE_SRC_SRCALPHA|GLSTATE_DST_ONEMINUSSRCALPHA|GLSTATE_DEPTHWRITE;
				stage->alphaGenType = ALPHAGEN_CONST;
				if (style & SHADER_TRANS33)
				{
					if (style & SHADER_TRANS66)
						stage->alphaGenConst = 0.22;	// both flags -- make it more translucent
					else
						stage->alphaGenConst = 0.33;
				}
				else if (style & SHADER_TRANS66)
					stage->alphaGenConst = 0.66;
			}
			if (!(stage->glState & (GLSTATE_SRCMASK|GLSTATE_DSTMASK)) && lightmapNumber == LIGHTMAP_NONE)
				stage->glState |= GLSTATE_DEPTHWRITE;

			if (style & SHADER_TURB)
			{
				tcModParms_t	*tcmod;
#if 0
				// scale
				tcmod = NewTcModStage (stage);
				tcmod->type = TCMOD_SCALE;
				tcmod->sScale = tcmod->tScale = 1.0f / 64;
				// turb
				tcmod = NewTcModStage (stage);
				tcmod->type = TCMOD_TURB;
				tcmod->wave.amp = 1.0f / 64;
				tcmod->wave.phase = 0;
				tcmod->wave.freq = 1.0f / (2.0f * M_PI);
#else
				tcmod = NewTcModStage (stage);
				tcmod->type = TCMOD_WARP;
#endif

#if 0
				{
					deformParms_t	*deform;

					// deform verts
					deform = &sh.deforms[0];
					deform->type = DEFORM_WAVE;
					deform->wave.type = FUNC_SIN;
					deform->wave.amp = 2;
					deform->wave.freq = 0.3;
					deform->wave.base = 0;
					deform->wave.phase = 0.5;
					deform->waveDiv = 1.0/(64*16);
					sh.numDeforms = 1;
				}
#endif
				// signal for map loader to subtesselate surface
				sh.tessSize = 64;
			}

			if (style & SHADER_SCROLL)
			{
				tcModParms_t *par;

				par = NewTcModStage (stage);
				par->type = TCMOD_SCROLL;
				if (style & SHADER_TURB)
					par->sSpeed = -0.5;
				else
					par->sSpeed = -64.0f / 40;
				par->tSpeed = 0;
			}

			if (!stage->rgbGenType)
			{
				if (style & SHADER_WALL)
					stage->rgbGenType = RGBGEN_IDENTITY_LIGHTING;	// world surface, not lightmapped (skins too ?? should use lighting)
				else
					stage->rgbGenType = RGBGEN_VERTEX;				// HUD image
			}
			if (!(style & SHADER_WALL))
				stage->glState = stage->glState & ~GLSTATE_DEPTHWRITE | GLSTATE_NODEPTHTEST;

			if (style & SHADER_ABSTRACT)
				st[0].tmu[0].numAnimTextures = 0;		// remove all stages

			/* CHECK:
				no lightmap: (skin)
					rgbGenFunc = LIGHTING_DIFFUSE
					alphaGenFunc = IDENTITY
					glState = DEPTHWRITE
				vertex lighting: (world)
					rgbGenFunc = EXACT_VERTEX
					alphaGenFunc = NONE
					glState = DEPTHWRITE
				game:
					rgbGenFunc = VERTEX
					alphaGenFunc = VERTEX
					glState = (SRC_ALPHA,ONE_MINUS_SRC_ALPHA)+NODEPTHTEST
				fullbright: (world)
				  st[0]:
					mapImage = *white
					rgbGenFunc = IDENTITY_LIGHTING
					alphaGenFunc = IDENTITY
					glState = DEPTHWRITE
				  st[1]:
					mapImage = texture
					rgbGenFunc = IDENTITY
					alphaGenFunc = IDENTITY
					glState = (DST_COLOR,ZERO)
				normal: (world)
				  st[0]:
					mapImage = lightmap
					rgbGenFunc = IDENTITY
					alphaGenFunc = IDENTITY
					glState = DEPTHWRITE
				  st[1]:
					mapImage = texture
					rgbGenFunc = IDENTITY
					alphaGenFunc = IDENTITY
					glState = (DST_COLOR,ZERO)
			*/
		}
	}
	return FinishShader ();
}


shader_t *GL_GetShaderByNum (int num)
{
	if (num >= 0 && num < shaderCount)
		return shadersArray[num];
	return gl_defaultShader;
}