#ifdef _WIN32
#  define NUM_GLFUNCS	66
#  define NUM_EXTFUNCS	16
#  define NUM_EXTENSIONS	8
#endif

#ifdef __linux__
#  define NUM_GLFUNCS	58
#  define NUM_EXTFUNCS	15
#  define NUM_EXTENSIONS	7
#endif

static char *qglNames[NUM_GLFUNCS + NUM_EXTFUNCS] = {
	"glAlphaFunc",
	"glArrayElement",
	"glBegin",
	"glBindTexture",
	"glBlendFunc",
	"glClear",
	"glClearColor",
	"glClearDepth",
	"glColor3f",
	"glColor3fv",
	"glColor4f",
	"glColor4fv",
	"glColor4ub",
	"glColor4ubv",
	"glColorPointer",
	"glCullFace",
	"glDeleteTextures",
	"glDepthFunc",
	"glDepthMask",
	"glDepthRange",
	"glDisable",
	"glDisableClientState",
	"glDrawBuffer",
	"glDrawElements",
	"glEnable",
	"glEnableClientState",
	"glEnd",
	"glFinish",
	"glFrustum",
	"glGetError",
	"glGetFloatv",
	"glGetIntegerv",
	"glGetString",
	"glLoadIdentity",
	"glLoadMatrixf",
	"glMatrixMode",
	"glOrtho",
	"glPolygonMode",
	"glPolygonOffset",
	"glPopMatrix",
	"glPushMatrix",
	"glReadPixels",
	"glRotatef",
	"glScalef",
	"glScissor",
	"glShadeModel",
	"glTexCoord2f",
	"glTexCoordPointer",
	"glTexEnvi",
	"glTexImage2D",
	"glTexParameterf",
	"glTexSubImage2D",
	"glTranslatef",
	"glVertex2f",
	"glVertex3f",
	"glVertex3fv",
	"glVertexPointer",
	"glViewport"
#ifdef _WIN32
,
	"wglChoosePixelFormat",
	"wglDescribePixelFormat",
	"wglSetPixelFormat",
	"wglSwapBuffers",
	"wglCreateContext",
	"wglDeleteContext",
	"wglGetProcAddress",
	"wglMakeCurrent"
#endif
,
	"glSelectTextureSGIS",
	"glMultiTexCoord2fSGIS",
	"glActiveTextureARB",
	"glClientActiveTextureARB",
	"glMultiTexCoord2fARB",
	"glLockArraysEXT",
	"glUnlockArraysEXT",
	"glGenFencesNV",
	"glDeleteFencesNV",
	"glSetFenceNV",
	"glFinishFenceNV"
#ifdef _WIN32
,
	"wglSwapIntervalEXT",
	"glVertexArrayRangeNV",
	"glFlushVertexArrayRangeNV",
	"wglAllocateMemoryNV",
	"wglFreeMemoryNV"
#endif

#ifdef __linux__
,
	"glVertexArrayRangeNV",
	"glFlushVertexArrayRangeNV",
	"glXAllocateMemoryNV",
	"glXFreeMemoryNV"
#endif

};

static char *EnumName (GLenum v)
{
	switch (v)
	{
	case GL_LINE_LOOP:
		return "LINE_LOOP";
	case GL_LINE_STRIP:
		return "LINE_STRIP";
	case GL_TRIANGLES:
		return "TRIANGLES";
	case GL_TRIANGLE_STRIP:
		return "TRIANGLE_STRIP";
	case GL_TRIANGLE_FAN:
		return "TRIANGLE_FAN";
	case GL_QUADS:
		return "QUADS";
	case GL_QUAD_STRIP:
		return "QUAD_STRIP";
	case GL_POLYGON:
		return "POLYGON";
	case GL_NEVER:
		return "NEVER";
	case GL_LESS:
		return "LESS";
	case GL_EQUAL:
		return "EQUAL";
	case GL_LEQUAL:
		return "LEQUAL";
	case GL_GREATER:
		return "GREATER";
	case GL_NOTEQUAL:
		return "NOTEQUAL";
	case GL_GEQUAL:
		return "GEQUAL";
	case GL_ALWAYS:
		return "ALWAYS";
	case GL_ZERO:
		return "ZERO";
	case GL_ONE:
		return "ONE";
	case GL_SRC_COLOR:
		return "SRC_COLOR";
	case GL_ONE_MINUS_SRC_COLOR:
		return "ONE_MINUS_SRC_COLOR";
	case GL_SRC_ALPHA:
		return "SRC_ALPHA";
	case GL_ONE_MINUS_SRC_ALPHA:
		return "ONE_MINUS_SRC_ALPHA";
	case GL_DST_ALPHA:
		return "DST_ALPHA";
	case GL_ONE_MINUS_DST_ALPHA:
		return "ONE_MINUS_DST_ALPHA";
	case GL_DST_COLOR:
		return "DST_COLOR";
	case GL_ONE_MINUS_DST_COLOR:
		return "ONE_MINUS_DST_COLOR";
	case GL_SRC_ALPHA_SATURATE:
		return "SRC_ALPHA_SATURATE";
	case GL_FRONT:
		return "FRONT";
	case GL_BACK:
		return "BACK";
	case GL_MODELVIEW:
		return "MODELVIEW";
	case GL_PROJECTION:
		return "PROJECTION";
	case GL_TEXTURE:
		return "TEXTURE";
	case GL_MODULATE:
		return "MODULATE";
	case GL_DECAL:
		return "DECAL";
	case GL_REPLACE:
		return "REPLACE";
	case GL_ADD:
		return "ADD";
	case GL_TEXTURE_ENV_MODE:
		return "TEXTURE_ENV_MODE";
	case GL_TEXTURE_ENV_COLOR:
		return "TEXTURE_ENV_COLOR";
	case GL_TEXTURE_ENV:
		return "TEXTURE_ENV";
	case GL_NEAREST:
		return "NEAREST";
	case GL_LINEAR:
		return "LINEAR";
	case GL_NEAREST_MIPMAP_NEAREST:
		return "NEAREST_MIPMAP_NEAREST";
	case GL_LINEAR_MIPMAP_NEAREST:
		return "LINEAR_MIPMAP_NEAREST";
	case GL_NEAREST_MIPMAP_LINEAR:
		return "NEAREST_MIPMAP_LINEAR";
	case GL_LINEAR_MIPMAP_LINEAR:
		return "LINEAR_MIPMAP_LINEAR";
	case GL_TEXTURE_MAG_FILTER:
		return "TEXTURE_MAG_FILTER";
	case GL_TEXTURE_MIN_FILTER:
		return "TEXTURE_MIN_FILTER";
	case GL_TEXTURE_WRAP_S:
		return "TEXTURE_WRAP_S";
	case GL_TEXTURE_WRAP_T:
		return "TEXTURE_WRAP_T";
	case GL_CLAMP:
		return "CLAMP";
	case GL_REPEAT:
		return "REPEAT";
	case GL_RGB5:
		return "RGB5";
	case GL_RGB8:
		return "RGB8";
	case GL_RGBA4:
		return "RGBA4";
	case GL_RGB5_A1:
		return "RGB5_A1";
	case GL_RGBA8:
		return "RGBA8";
	case GL_RGB4_S3TC:
		return "RGB4_S3TC";
	case GL_COMPRESSED_RGB_ARB:
		return "COMPRESSED_RGB_ARB";
	case GL_COMPRESSED_RGBA_ARB:
		return "COMPRESSED_RGBA_ARB";
	case GL_ACTIVE_TEXTURE_ARB:
		return "ACTIVE_TEXTURE_ARB";
	case GL_CLIENT_ACTIVE_TEXTURE_ARB:
		return "CLIENT_ACTIVE_TEXTURE_ARB";
	case GL_MAX_TEXTURE_UNITS_ARB:
		return "MAX_TEXTURE_UNITS_ARB";
	case GL_TEXTURE0_ARB:
		return "TEXTURE0_ARB";
	case GL_TEXTURE1_ARB:
		return "TEXTURE1_ARB";
	case GL_MAX_TEXTURES_SGIS:
		return "MAX_TEXTURES_SGIS";
	case GL_TEXTURE0_SGIS:
		return "TEXTURE0_SGIS";
	case GL_TEXTURE1_SGIS:
		return "TEXTURE1_SGIS";
	case GL_FOG:
		return "FOG";
	case GL_TEXTURE_2D:
		return "TEXTURE_2D";
	case GL_CULL_FACE:
		return "CULL_FACE";
	case GL_ALPHA_TEST:
		return "ALPHA_TEST";
	case GL_BLEND:
		return "BLEND";
	case GL_DEPTH_TEST:
		return "DEPTH_TEST";
	case GL_VERTEX_ARRAY:
		return "VERTEX_ARRAY";
	case GL_COLOR_ARRAY:
		return "COLOR_ARRAY";
	case GL_TEXTURE_COORD_ARRAY:
		return "TEXTURE_COORD_ARRAY";
	case GL_BYTE:
		return "BYTE";
	case GL_UNSIGNED_BYTE:
		return "UNSIGNED_BYTE";
	case GL_SHORT:
		return "SHORT";
	case GL_UNSIGNED_SHORT:
		return "UNSIGNED_SHORT";
	case GL_INT:
		return "INT";
	case GL_UNSIGNED_INT:
		return "UNSIGNED_INT";
	case GL_FLOAT:
		return "FLOAT";
	case GL_DOUBLE:
		return "DOUBLE";
	default:
		return "???";
	}
}

static void APIENTRY logAlphaFunc (GLenum func, GLclampf ref)
{
	fprintf (glw_state.log_fp, "%s (GL_%s, %g)\n", "glAlphaFunc", EnumName(func), ref);
	lib.AlphaFunc (func, ref);
}

static void APIENTRY logArrayElement (GLint i)
{
	fprintf (glw_state.log_fp, "%s (%d)\n", "glArrayElement", i);
	lib.ArrayElement (i);
}

static void APIENTRY logBegin (GLenum mode)
{
	fprintf (glw_state.log_fp, "%s (GL_%s)\n", "glBegin", EnumName(mode));
	lib.Begin (mode);
}

static void APIENTRY logBindTexture (GLenum target, GLuint texture)
{
	fprintf (glw_state.log_fp, "%s (GL_%s, %d)\n", "glBindTexture", EnumName(target), texture);
	lib.BindTexture (target, texture);
}

static void APIENTRY logBlendFunc (GLenum sfactor, GLenum dfactor)
{
	fprintf (glw_state.log_fp, "%s (GL_%s, GL_%s)\n", "glBlendFunc", EnumName(sfactor), EnumName(dfactor));
	lib.BlendFunc (sfactor, dfactor);
}

static void APIENTRY logClear (GLbitfield mask)
{
	fprintf (glw_state.log_fp, "%s\n", "glClear");
	lib.Clear (mask);
}

static void APIENTRY logClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	fprintf (glw_state.log_fp, "%s (%g, %g, %g, %g)\n", "glClearColor", red, green, blue, alpha);
	lib.ClearColor (red, green, blue, alpha);
}

static void APIENTRY logClearDepth (GLclampd depth)
{
	fprintf (glw_state.log_fp, "%s (%g)\n", "glClearDepth", depth);
	lib.ClearDepth (depth);
}

static void APIENTRY logColor3f (GLfloat red, GLfloat green, GLfloat blue)
{
	fprintf (glw_state.log_fp, "%s (%g, %g, %g)\n", "glColor3f", red, green, blue);
	lib.Color3f (red, green, blue);
}

static void APIENTRY logColor3fv (const GLfloat *v)
{
	fprintf (glw_state.log_fp, "%s\n", "glColor3fv");
	lib.Color3fv (v);
}

static void APIENTRY logColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	fprintf (glw_state.log_fp, "%s (%g, %g, %g, %g)\n", "glColor4f", red, green, blue, alpha);
	lib.Color4f (red, green, blue, alpha);
}

static void APIENTRY logColor4fv (const GLfloat *v)
{
	fprintf (glw_state.log_fp, "%s\n", "glColor4fv");
	lib.Color4fv (v);
}

static void APIENTRY logColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
	fprintf (glw_state.log_fp, "%s (%d, %d, %d, %d)\n", "glColor4ub", red, green, blue, alpha);
	lib.Color4ub (red, green, blue, alpha);
}

static void APIENTRY logColor4ubv (const GLubyte *v)
{
	fprintf (glw_state.log_fp, "%s\n", "glColor4ubv");
	lib.Color4ubv (v);
}

static void APIENTRY logColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	fprintf (glw_state.log_fp, "%s (%d, GL_%s, %d, $%X)\n", "glColorPointer", size, EnumName(type), stride, pointer);
	lib.ColorPointer (size, type, stride, pointer);
}

static void APIENTRY logCullFace (GLenum mode)
{
	fprintf (glw_state.log_fp, "%s (GL_%s)\n", "glCullFace", EnumName(mode));
	lib.CullFace (mode);
}

static void APIENTRY logDeleteTextures (GLsizei n, const GLuint *textures)
{
	fprintf (glw_state.log_fp, "%s (%d, %d)\n", "glDeleteTextures", n, textures);
	lib.DeleteTextures (n, textures);
}

static void APIENTRY logDepthFunc (GLenum func)
{
	fprintf (glw_state.log_fp, "%s (GL_%s)\n", "glDepthFunc", EnumName(func));
	lib.DepthFunc (func);
}

static void APIENTRY logDepthMask (GLboolean flag)
{
	fprintf (glw_state.log_fp, "%s (%d)\n", "glDepthMask", flag);
	lib.DepthMask (flag);
}

static void APIENTRY logDepthRange (GLclampd zNear, GLclampd zFar)
{
	fprintf (glw_state.log_fp, "%s (%g, %g)\n", "glDepthRange", zNear, zFar);
	lib.DepthRange (zNear, zFar);
}

static void APIENTRY logDisable (GLenum cap)
{
	fprintf (glw_state.log_fp, "%s (GL_%s)\n", "glDisable", EnumName(cap));
	lib.Disable (cap);
}

static void APIENTRY logDisableClientState (GLenum array)
{
	fprintf (glw_state.log_fp, "%s (GL_%s)\n", "glDisableClientState", EnumName(array));
	lib.DisableClientState (array);
}

static void APIENTRY logDrawBuffer (GLenum mode)
{
	fprintf (glw_state.log_fp, "%s (GL_%s)\n", "glDrawBuffer", EnumName(mode));
	lib.DrawBuffer (mode);
}

static void APIENTRY logDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
	fprintf (glw_state.log_fp, "%s (GL_%s, %d, GL_%s, $%X)\n", "glDrawElements", EnumName(mode), count, EnumName(type), indices);
	lib.DrawElements (mode, count, type, indices);
}

static void APIENTRY logEnable (GLenum cap)
{
	fprintf (glw_state.log_fp, "%s (GL_%s)\n", "glEnable", EnumName(cap));
	lib.Enable (cap);
}

static void APIENTRY logEnableClientState (GLenum array)
{
	fprintf (glw_state.log_fp, "%s (GL_%s)\n", "glEnableClientState", EnumName(array));
	lib.EnableClientState (array);
}

static void APIENTRY logEnd (void)
{
	fprintf (glw_state.log_fp, "%s\n", "glEnd");
	lib.End ();
}

static void APIENTRY logFinish (void)
{
	fprintf (glw_state.log_fp, "%s\n", "glFinish");
	lib.Finish ();
}

static void APIENTRY logFrustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	fprintf (glw_state.log_fp, "%s (%g, %g, %g, %g, %g, %g)\n", "glFrustum", left, right, bottom, top, zNear, zFar);
	lib.Frustum (left, right, bottom, top, zNear, zFar);
}

static GLenum APIENTRY logGetError (void)
{
	fprintf (glw_state.log_fp, "%s\n", "glGetError");
	return lib.GetError ();
}

static void APIENTRY logGetFloatv (GLenum pname, GLfloat *params)
{
	fprintf (glw_state.log_fp, "%s\n", "glGetFloatv");
	lib.GetFloatv (pname, params);
}

static void APIENTRY logGetIntegerv (GLenum pname, GLint *params)
{
	fprintf (glw_state.log_fp, "%s\n", "glGetIntegerv");
	lib.GetIntegerv (pname, params);
}

static const GLubyte * APIENTRY logGetString (GLenum name)
{
	fprintf (glw_state.log_fp, "%s\n", "glGetString");
	return lib.GetString (name);
}

static void APIENTRY logLoadIdentity (void)
{
	fprintf (glw_state.log_fp, "%s\n", "glLoadIdentity");
	lib.LoadIdentity ();
}

static void APIENTRY logLoadMatrixf (const GLfloat *m)
{
	fprintf (glw_state.log_fp, "%s\n", "glLoadMatrixf");
	lib.LoadMatrixf (m);
}

static void APIENTRY logMatrixMode (GLenum mode)
{
	fprintf (glw_state.log_fp, "%s (GL_%s)\n", "glMatrixMode", EnumName(mode));
	lib.MatrixMode (mode);
}

static void APIENTRY logOrtho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	fprintf (glw_state.log_fp, "%s (%g, %g, %g, %g, %g, %g)\n", "glOrtho", left, right, bottom, top, zNear, zFar);
	lib.Ortho (left, right, bottom, top, zNear, zFar);
}

static void APIENTRY logPolygonMode (GLenum face, GLenum mode)
{
	fprintf (glw_state.log_fp, "%s (GL_%s, GL_%s)\n", "glPolygonMode", EnumName(face), EnumName(mode));
	lib.PolygonMode (face, mode);
}

static void APIENTRY logPolygonOffset (GLfloat factor, GLfloat units)
{
	fprintf (glw_state.log_fp, "%s (%g, %g)\n", "glPolygonOffset", factor, units);
	lib.PolygonOffset (factor, units);
}

static void APIENTRY logPopMatrix (void)
{
	fprintf (glw_state.log_fp, "%s\n", "glPopMatrix");
	lib.PopMatrix ();
}

static void APIENTRY logPushMatrix (void)
{
	fprintf (glw_state.log_fp, "%s\n", "glPushMatrix");
	lib.PushMatrix ();
}

static void APIENTRY logReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
	fprintf (glw_state.log_fp, "%s (%d, %d, %d, %d, GL_%s, GL_%s, $%X)\n", "glReadPixels", x, y, width, height, EnumName(format), EnumName(type), pixels);
	lib.ReadPixels (x, y, width, height, format, type, pixels);
}

static void APIENTRY logRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	fprintf (glw_state.log_fp, "%s (%g, %g, %g, %g)\n", "glRotatef", angle, x, y, z);
	lib.Rotatef (angle, x, y, z);
}

static void APIENTRY logScalef (GLfloat x, GLfloat y, GLfloat z)
{
	fprintf (glw_state.log_fp, "%s (%g, %g, %g)\n", "glScalef", x, y, z);
	lib.Scalef (x, y, z);
}

static void APIENTRY logScissor (GLint x, GLint y, GLsizei width, GLsizei height)
{
	fprintf (glw_state.log_fp, "%s (%d, %d, %d, %d)\n", "glScissor", x, y, width, height);
	lib.Scissor (x, y, width, height);
}

static void APIENTRY logShadeModel (GLenum mode)
{
	fprintf (glw_state.log_fp, "%s (GL_%s)\n", "glShadeModel", EnumName(mode));
	lib.ShadeModel (mode);
}

static void APIENTRY logTexCoord2f (GLfloat s, GLfloat t)
{
	fprintf (glw_state.log_fp, "%s (%g, %g)\n", "glTexCoord2f", s, t);
	lib.TexCoord2f (s, t);
}

static void APIENTRY logTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	fprintf (glw_state.log_fp, "%s (%d, GL_%s, %d, $%X)\n", "glTexCoordPointer", size, EnumName(type), stride, pointer);
	lib.TexCoordPointer (size, type, stride, pointer);
}

static void APIENTRY logTexEnvi (GLenum target, GLenum pname, GLint param)
{
	fprintf (glw_state.log_fp, "%s (GL_%s, GL_%s, %d)\n", "glTexEnvi", EnumName(target), EnumName(pname), param);
	lib.TexEnvi (target, pname, param);
}

static void APIENTRY logTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
	fprintf (glw_state.log_fp, "%s (GL_%s, %d, %d, %d, %d, %d, GL_%s, GL_%s, $%X)\n", "glTexImage2D", EnumName(target), level, internalformat, width, height, border, EnumName(format), EnumName(type), pixels);
	lib.TexImage2D (target, level, internalformat, width, height, border, format, type, pixels);
}

static void APIENTRY logTexParameterf (GLenum target, GLenum pname, GLfloat param)
{
	fprintf (glw_state.log_fp, "%s (GL_%s, GL_%s, %g)\n", "glTexParameterf", EnumName(target), EnumName(pname), param);
	lib.TexParameterf (target, pname, param);
}

static void APIENTRY logTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
	fprintf (glw_state.log_fp, "%s (GL_%s, %d, %d, %d, %d, %d, GL_%s, GL_%s, $%X)\n", "glTexSubImage2D", EnumName(target), level, xoffset, yoffset, width, height, EnumName(format), EnumName(type), pixels);
	lib.TexSubImage2D (target, level, xoffset, yoffset, width, height, format, type, pixels);
}

static void APIENTRY logTranslatef (GLfloat x, GLfloat y, GLfloat z)
{
	fprintf (glw_state.log_fp, "%s (%g, %g, %g)\n", "glTranslatef", x, y, z);
	lib.Translatef (x, y, z);
}

static void APIENTRY logVertex2f (GLfloat x, GLfloat y)
{
	fprintf (glw_state.log_fp, "%s (%g, %g)\n", "glVertex2f", x, y);
	lib.Vertex2f (x, y);
}

static void APIENTRY logVertex3f (GLfloat x, GLfloat y, GLfloat z)
{
	fprintf (glw_state.log_fp, "%s (%g, %g, %g)\n", "glVertex3f", x, y, z);
	lib.Vertex3f (x, y, z);
}

static void APIENTRY logVertex3fv (const GLfloat *v)
{
	fprintf (glw_state.log_fp, "%s\n", "glVertex3fv");
	lib.Vertex3fv (v);
}

static void APIENTRY logVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	fprintf (glw_state.log_fp, "%s (%d, GL_%s, %d, $%X)\n", "glVertexPointer", size, EnumName(type), stride, pointer);
	lib.VertexPointer (size, type, stride, pointer);
}

static void APIENTRY logViewport (GLint x, GLint y, GLsizei width, GLsizei height)
{
	fprintf (glw_state.log_fp, "%s (%d, %d, %d, %d)\n", "glViewport", x, y, width, height);
	lib.Viewport (x, y, width, height);
}


#ifdef _WIN32

static int APIENTRY logwglChoosePixelFormat (HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd)
{
	fprintf (glw_state.log_fp, "%s\n", "wglChoosePixelFormat");
	return lib.wglChoosePixelFormat (hdc, ppfd);
}

static int APIENTRY logwglDescribePixelFormat (HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
{
	fprintf (glw_state.log_fp, "%s\n", "wglDescribePixelFormat");
	return lib.wglDescribePixelFormat (hdc, iPixelFormat, nBytes, ppfd);
}

static BOOL APIENTRY logwglSetPixelFormat (HDC hdc, int iPixelFormat, CONST PIXELFORMATDESCRIPTOR *ppfd)
{
	fprintf (glw_state.log_fp, "%s\n", "wglSetPixelFormat");
	return lib.wglSetPixelFormat (hdc, iPixelFormat, ppfd);
}

static BOOL APIENTRY logwglSwapBuffers (HDC hdc)
{
	fprintf (glw_state.log_fp, "%s\n", "wglSwapBuffers");
	return lib.wglSwapBuffers (hdc);
}

static HGLRC APIENTRY logwglCreateContext (HDC hdc)
{
	fprintf (glw_state.log_fp, "%s\n", "wglCreateContext");
	return lib.wglCreateContext (hdc);
}

static BOOL APIENTRY logwglDeleteContext (HGLRC hglrc)
{
	fprintf (glw_state.log_fp, "%s\n", "wglDeleteContext");
	return lib.wglDeleteContext (hglrc);
}

static PROC APIENTRY logwglGetProcAddress (LPCSTR lpszProc)
{
	fprintf (glw_state.log_fp, "%s\n", "wglGetProcAddress");
	return lib.wglGetProcAddress (lpszProc);
}

static BOOL APIENTRY logwglMakeCurrent (HDC hdc, HGLRC hglrc)
{
	fprintf (glw_state.log_fp, "%s\n", "wglMakeCurrent");
	return lib.wglMakeCurrent (hdc, hglrc);
}


#endif

static void APIENTRY logSelectTextureSGIS (GLenum target)
{
	fprintf (glw_state.log_fp, "%s (GL_%s)\n", "glSelectTextureSGIS", EnumName(target));
	lib.SelectTextureSGIS (target);
}

static void APIENTRY logMultiTexCoord2fSGIS (GLenum target, GLfloat s, GLfloat t)
{
	fprintf (glw_state.log_fp, "%s (GL_%s, %g, %g)\n", "glMultiTexCoord2fSGIS", EnumName(target), s, t);
	lib.MultiTexCoord2fSGIS (target, s, t);
}

static void APIENTRY logActiveTextureARB (GLenum texture)
{
	fprintf (glw_state.log_fp, "%s (GL_%s)\n", "glActiveTextureARB", EnumName(texture));
	lib.ActiveTextureARB (texture);
}

static void APIENTRY logClientActiveTextureARB (GLenum texture)
{
	fprintf (glw_state.log_fp, "%s (GL_%s)\n", "glClientActiveTextureARB", EnumName(texture));
	lib.ClientActiveTextureARB (texture);
}

static void APIENTRY logMultiTexCoord2fARB (GLenum target, GLfloat s, GLfloat t)
{
	fprintf (glw_state.log_fp, "%s (GL_%s, %g, %g)\n", "glMultiTexCoord2fARB", EnumName(target), s, t);
	lib.MultiTexCoord2fARB (target, s, t);
}

static void APIENTRY logLockArraysEXT (GLint first, GLint count)
{
	fprintf (glw_state.log_fp, "%s (%d, %d)\n", "glLockArraysEXT", first, count);
	lib.LockArraysEXT (first, count);
}

static void APIENTRY logUnlockArraysEXT (void)
{
	fprintf (glw_state.log_fp, "%s\n", "glUnlockArraysEXT");
	lib.UnlockArraysEXT ();
}

static void APIENTRY logGenFencesNV (GLint n, GLuint *fences)
{
	fprintf (glw_state.log_fp, "%s\n", "glGenFencesNV");
	lib.GenFencesNV (n, fences);
}

static void APIENTRY logDeleteFencesNV (GLint n, const GLuint *fences)
{
	fprintf (glw_state.log_fp, "%s\n", "glDeleteFencesNV");
	lib.DeleteFencesNV (n, fences);
}

static void APIENTRY logSetFenceNV (GLuint fence, GLenum condition)
{
	fprintf (glw_state.log_fp, "%s\n", "glSetFenceNV");
	lib.SetFenceNV (fence, condition);
}

static void APIENTRY logFinishFenceNV (GLuint fence)
{
	fprintf (glw_state.log_fp, "%s\n", "glFinishFenceNV");
	lib.FinishFenceNV (fence);
}


#ifdef _WIN32

static BOOL APIENTRY logwglSwapIntervalEXT (int interval)
{
	fprintf (glw_state.log_fp, "%s (%d)\n", "wglSwapIntervalEXT", interval);
	return lib.wglSwapIntervalEXT (interval);
}

static void APIENTRY logglVertexArrayRangeNV (GLint lenght, GLvoid *pointer)
{
	fprintf (glw_state.log_fp, "%s\n", "glVertexArrayRangeNV");
	lib.glVertexArrayRangeNV (lenght, pointer);
}

static void APIENTRY logglFlushVertexArrayRangeNV (void)
{
	fprintf (glw_state.log_fp, "%s\n", "glFlushVertexArrayRangeNV");
	lib.glFlushVertexArrayRangeNV ();
}

static void* APIENTRY logglAllocateMemoryNV (int size, float readFreq, float writeFreq, float prior)
{
	fprintf (glw_state.log_fp, "%s\n", "glAllocateMemoryNV");
	return lib.glAllocateMemoryNV (size, readFreq, writeFreq, prior);
}

static void APIENTRY logglFreeMemoryNV (void *pointer)
{
	fprintf (glw_state.log_fp, "%s\n", "glFreeMemoryNV");
	lib.glFreeMemoryNV (pointer);
}


#endif


#ifdef __linux__

static void APIENTRY logglVertexArrayRangeNV (GLint lenght, GLvoid *pointer)
{
	fprintf (glw_state.log_fp, "%s\n", "glVertexArrayRangeNV");
	lib.glVertexArrayRangeNV (lenght, pointer);
}

static void APIENTRY logglFlushVertexArrayRangeNV (void)
{
	fprintf (glw_state.log_fp, "%s\n", "glFlushVertexArrayRangeNV");
	lib.glFlushVertexArrayRangeNV ();
}

static void* APIENTRY logglAllocateMemoryNV (int size, float readFreq, float writeFreq, float prior)
{
	fprintf (glw_state.log_fp, "%s\n", "glAllocateMemoryNV");
	return lib.glAllocateMemoryNV (size, readFreq, writeFreq, prior);
}

static void APIENTRY logglFreeMemoryNV (void *pointer)
{
	fprintf (glw_state.log_fp, "%s\n", "glFreeMemoryNV");
	lib.glFreeMemoryNV (pointer);
}


#endif

static qgl_t logFuncs = {
	logAlphaFunc,
	logArrayElement,
	logBegin,
	logBindTexture,
	logBlendFunc,
	logClear,
	logClearColor,
	logClearDepth,
	logColor3f,
	logColor3fv,
	logColor4f,
	logColor4fv,
	logColor4ub,
	logColor4ubv,
	logColorPointer,
	logCullFace,
	logDeleteTextures,
	logDepthFunc,
	logDepthMask,
	logDepthRange,
	logDisable,
	logDisableClientState,
	logDrawBuffer,
	logDrawElements,
	logEnable,
	logEnableClientState,
	logEnd,
	logFinish,
	logFrustum,
	logGetError,
	logGetFloatv,
	logGetIntegerv,
	logGetString,
	logLoadIdentity,
	logLoadMatrixf,
	logMatrixMode,
	logOrtho,
	logPolygonMode,
	logPolygonOffset,
	logPopMatrix,
	logPushMatrix,
	logReadPixels,
	logRotatef,
	logScalef,
	logScissor,
	logShadeModel,
	logTexCoord2f,
	logTexCoordPointer,
	logTexEnvi,
	logTexImage2D,
	logTexParameterf,
	logTexSubImage2D,
	logTranslatef,
	logVertex2f,
	logVertex3f,
	logVertex3fv,
	logVertexPointer,
	logViewport
#ifdef _WIN32
,
	logwglChoosePixelFormat,
	logwglDescribePixelFormat,
	logwglSetPixelFormat,
	logwglSwapBuffers,
	logwglCreateContext,
	logwglDeleteContext,
	logwglGetProcAddress,
	logwglMakeCurrent
#endif
,
	logSelectTextureSGIS,
	logMultiTexCoord2fSGIS,
	logActiveTextureARB,
	logClientActiveTextureARB,
	logMultiTexCoord2fARB,
	logLockArraysEXT,
	logUnlockArraysEXT,
	logGenFencesNV,
	logDeleteFencesNV,
	logSetFenceNV,
	logFinishFenceNV
#ifdef _WIN32
,
	logwglSwapIntervalEXT,
	logglVertexArrayRangeNV,
	logglFlushVertexArrayRangeNV,
	logglAllocateMemoryNV,
	logglFreeMemoryNV
#endif

#ifdef __linux__
,
	logglVertexArrayRangeNV,
	logglFlushVertexArrayRangeNV,
	logglAllocateMemoryNV,
	logglFreeMemoryNV
#endif

};

typedef struct {
	char	*name;
	char	*cvar;
	short	first, count;
	int		require, deprecate;
} extInfo_t;

static extInfo_t extInfo[NUM_EXTENSIONS] = {
	{"GL_SGIS_multitexture", "gl_ext_multitexture", NUM_GLFUNCS+0, 2, 0, QGL_ARB_MULTITEXTURE},
	{"GL_ARB_multitexture", "gl_ext_multitexture", NUM_GLFUNCS+2, 3, 0, 0},
	{"GL_S3_s3tc", "gl_ext_compressed_textures", 0, 0, 0, QGL_ARB_TEXTURE_COMPRESSION},
	{"GL_ARB_texture_compression", "gl_ext_compressed_textures", 0, 0, 0, 0},
	{"GL_EXT_compiled_vertex_array", "gl_ext_compiled_vertex_array", NUM_GLFUNCS+5, 2, 0, QGL_NV_VERTEX_ARRAY_RANGE},
	{"GL_NV_fence", "gl_ext_vertex_array_range", NUM_GLFUNCS+7, 4, 0, 0}
#ifdef _WIN32
,
	{"WGL_EXT_swap_control", NULL, NUM_GLFUNCS+11, 1, 0, 0},
	{"GL_NV_vertex_array_range", "gl_ext_vertex_array_range", NUM_GLFUNCS+12, 4, QGL_NV_FENCE, 0}
#endif

#ifdef __linux__
,
	{"GL_NV_vertex_array_range", "gl_ext_vertex_array_range", NUM_GLFUNCS+11, 4, QGL_NV_FENCE, 0}
#endif
};

