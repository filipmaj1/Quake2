typedef void (APIENTRY * dummyFunc_t) (void);

typedef union
{
	struct {
		void	(APIENTRY * AlphaFunc) (GLenum func, GLclampf ref);
		void	(APIENTRY * ArrayElement) (GLint i);
		void	(APIENTRY * Begin) (GLenum mode);
		void	(APIENTRY * BindTexture) (GLenum target, GLuint texture);
		void	(APIENTRY * BlendFunc) (GLenum sfactor, GLenum dfactor);
		void	(APIENTRY * Clear) (GLbitfield mask);
		void	(APIENTRY * ClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
		void	(APIENTRY * ClearDepth) (GLclampd depth);
		void	(APIENTRY * Color3f) (GLfloat red, GLfloat green, GLfloat blue);
		void	(APIENTRY * Color3fv) (const GLfloat *v);
		void	(APIENTRY * Color4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		void	(APIENTRY * Color4fv) (const GLfloat *v);
		void	(APIENTRY * Color4ub) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
		void	(APIENTRY * Color4ubv) (const GLubyte *v);
		void	(APIENTRY * ColorPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
		void	(APIENTRY * CullFace) (GLenum mode);
		void	(APIENTRY * DeleteTextures) (GLsizei n, const GLuint *textures);
		void	(APIENTRY * DepthFunc) (GLenum func);
		void	(APIENTRY * DepthMask) (GLboolean flag);
		void	(APIENTRY * DepthRange) (GLclampd zNear, GLclampd zFar);
		void	(APIENTRY * Disable) (GLenum cap);
		void	(APIENTRY * DisableClientState) (GLenum array);
		void	(APIENTRY * DrawBuffer) (GLenum mode);
		void	(APIENTRY * DrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
		void	(APIENTRY * Enable) (GLenum cap);
		void	(APIENTRY * EnableClientState) (GLenum array);
		void	(APIENTRY * End) (void);
		void	(APIENTRY * Finish) (void);
		void	(APIENTRY * Frustum) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
		GLenum	(APIENTRY * GetError) (void);
		void	(APIENTRY * GetFloatv) (GLenum pname, GLfloat *params);
		void	(APIENTRY * GetIntegerv) (GLenum pname, GLint *params);
		const GLubyte *	(APIENTRY * GetString) (GLenum name);
		void	(APIENTRY * LoadIdentity) (void);
		void	(APIENTRY * LoadMatrixf) (const GLfloat *m);
		void	(APIENTRY * MatrixMode) (GLenum mode);
		void	(APIENTRY * Ortho) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
		void	(APIENTRY * PolygonMode) (GLenum face, GLenum mode);
		void	(APIENTRY * PolygonOffset) (GLfloat factor, GLfloat units);
		void	(APIENTRY * PopMatrix) (void);
		void	(APIENTRY * PushMatrix) (void);
		void	(APIENTRY * ReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
		void	(APIENTRY * Rotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
		void	(APIENTRY * Scalef) (GLfloat x, GLfloat y, GLfloat z);
		void	(APIENTRY * Scissor) (GLint x, GLint y, GLsizei width, GLsizei height);
		void	(APIENTRY * ShadeModel) (GLenum mode);
		void	(APIENTRY * TexCoord2f) (GLfloat s, GLfloat t);
		void	(APIENTRY * TexCoordPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
		void	(APIENTRY * TexEnvi) (GLenum target, GLenum pname, GLint param);
		void	(APIENTRY * TexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
		void	(APIENTRY * TexParameterf) (GLenum target, GLenum pname, GLfloat param);
		void	(APIENTRY * TexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
		void	(APIENTRY * Translatef) (GLfloat x, GLfloat y, GLfloat z);
		void	(APIENTRY * Vertex2f) (GLfloat x, GLfloat y);
		void	(APIENTRY * Vertex3f) (GLfloat x, GLfloat y, GLfloat z);
		void	(APIENTRY * Vertex3fv) (const GLfloat *v);
		void	(APIENTRY * VertexPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
		void	(APIENTRY * Viewport) (GLint x, GLint y, GLsizei width, GLsizei height);

#ifdef _WIN32
		int	(APIENTRY * wglChoosePixelFormat) (HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd);
		int	(APIENTRY * wglDescribePixelFormat) (HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd);
		BOOL	(APIENTRY * wglSetPixelFormat) (HDC hdc, int iPixelFormat, CONST PIXELFORMATDESCRIPTOR *ppfd);
		BOOL	(APIENTRY * wglSwapBuffers) (HDC hdc);
		HGLRC	(APIENTRY * wglCreateContext) (HDC hdc);
		BOOL	(APIENTRY * wglDeleteContext) (HGLRC hglrc);
		PROC	(APIENTRY * wglGetProcAddress) (LPCSTR lpszProc);
		BOOL	(APIENTRY * wglMakeCurrent) (HDC hdc, HGLRC hglrc);
#endif
		void	(APIENTRY * SelectTextureSGIS) (GLenum target);
		void	(APIENTRY * MultiTexCoord2fSGIS) (GLenum target, GLfloat s, GLfloat t);
		void	(APIENTRY * ActiveTextureARB) (GLenum texture);
		void	(APIENTRY * ClientActiveTextureARB) (GLenum texture);
		void	(APIENTRY * MultiTexCoord2fARB) (GLenum target, GLfloat s, GLfloat t);
		void	(APIENTRY * LockArraysEXT) (GLint first, GLint count);
		void	(APIENTRY * UnlockArraysEXT) (void);
		void	(APIENTRY * GenFencesNV) (GLint n, GLuint *fences);
		void	(APIENTRY * DeleteFencesNV) (GLint n, const GLuint *fences);
		void	(APIENTRY * SetFenceNV) (GLuint fence, GLenum condition);
		void	(APIENTRY * FinishFenceNV) (GLuint fence);

#ifdef _WIN32
		BOOL	(APIENTRY * wglSwapIntervalEXT) (int interval);
		void	(APIENTRY * glVertexArrayRangeNV) (GLint lenght, GLvoid *pointer);
		void	(APIENTRY * glFlushVertexArrayRangeNV) (void);
		void*	(APIENTRY * glAllocateMemoryNV) (int size, float readFreq, float writeFreq, float prior);
		void	(APIENTRY * glFreeMemoryNV) (void *pointer);
#endif

#ifdef __linux__
		void	(APIENTRY * glVertexArrayRangeNV) (GLint lenght, GLvoid *pointer);
		void	(APIENTRY * glFlushVertexArrayRangeNV) (void);
		void*	(APIENTRY * glAllocateMemoryNV) (int size, float readFreq, float writeFreq, float prior);
		void	(APIENTRY * glFreeMemoryNV) (void *pointer);
#endif
	};
	dummyFunc_t funcs[1];
} qgl_t;

extern qgl_t qgl;

#define qglAlphaFunc	qgl.AlphaFunc
#define qglArrayElement	qgl.ArrayElement
#define qglBegin	qgl.Begin
#define qglBindTexture	qgl.BindTexture
#define qglBlendFunc	qgl.BlendFunc
#define qglClear	qgl.Clear
#define qglClearColor	qgl.ClearColor
#define qglClearDepth	qgl.ClearDepth
#define qglColor3f	qgl.Color3f
#define qglColor3fv	qgl.Color3fv
#define qglColor4f	qgl.Color4f
#define qglColor4fv	qgl.Color4fv
#define qglColor4ub	qgl.Color4ub
#define qglColor4ubv	qgl.Color4ubv
#define qglColorPointer	qgl.ColorPointer
#define qglCullFace	qgl.CullFace
#define qglDeleteTextures	qgl.DeleteTextures
#define qglDepthFunc	qgl.DepthFunc
#define qglDepthMask	qgl.DepthMask
#define qglDepthRange	qgl.DepthRange
#define qglDisable	qgl.Disable
#define qglDisableClientState	qgl.DisableClientState
#define qglDrawBuffer	qgl.DrawBuffer
#define qglDrawElements	qgl.DrawElements
#define qglEnable	qgl.Enable
#define qglEnableClientState	qgl.EnableClientState
#define qglEnd	qgl.End
#define qglFinish	qgl.Finish
#define qglFrustum	qgl.Frustum
#define qglGetError	qgl.GetError
#define qglGetFloatv	qgl.GetFloatv
#define qglGetIntegerv	qgl.GetIntegerv
#define qglGetString	qgl.GetString
#define qglLoadIdentity	qgl.LoadIdentity
#define qglLoadMatrixf	qgl.LoadMatrixf
#define qglMatrixMode	qgl.MatrixMode
#define qglOrtho	qgl.Ortho
#define qglPolygonMode	qgl.PolygonMode
#define qglPolygonOffset	qgl.PolygonOffset
#define qglPopMatrix	qgl.PopMatrix
#define qglPushMatrix	qgl.PushMatrix
#define qglReadPixels	qgl.ReadPixels
#define qglRotatef	qgl.Rotatef
#define qglScalef	qgl.Scalef
#define qglScissor	qgl.Scissor
#define qglShadeModel	qgl.ShadeModel
#define qglTexCoord2f	qgl.TexCoord2f
#define qglTexCoordPointer	qgl.TexCoordPointer
#define qglTexEnvi	qgl.TexEnvi
#define qglTexImage2D	qgl.TexImage2D
#define qglTexParameterf	qgl.TexParameterf
#define qglTexSubImage2D	qgl.TexSubImage2D
#define qglTranslatef	qgl.Translatef
#define qglVertex2f	qgl.Vertex2f
#define qglVertex3f	qgl.Vertex3f
#define qglVertex3fv	qgl.Vertex3fv
#define qglVertexPointer	qgl.VertexPointer
#define qglViewport	qgl.Viewport

#ifdef _WIN32
#define qwglChoosePixelFormat	qgl.wglChoosePixelFormat
#define qwglDescribePixelFormat	qgl.wglDescribePixelFormat
#define qwglSetPixelFormat	qgl.wglSetPixelFormat
#define qwglSwapBuffers	qgl.wglSwapBuffers
#define qwglCreateContext	qgl.wglCreateContext
#define qwglDeleteContext	qgl.wglDeleteContext
#define qwglGetProcAddress	qgl.wglGetProcAddress
#define qwglMakeCurrent	qgl.wglMakeCurrent
#endif
#define qglSelectTextureSGIS	qgl.SelectTextureSGIS
#define qglMultiTexCoord2fSGIS	qgl.MultiTexCoord2fSGIS
#define qglActiveTextureARB	qgl.ActiveTextureARB
#define qglClientActiveTextureARB	qgl.ClientActiveTextureARB
#define qglMultiTexCoord2fARB	qgl.MultiTexCoord2fARB
#define qglLockArraysEXT	qgl.LockArraysEXT
#define qglUnlockArraysEXT	qgl.UnlockArraysEXT
#define qglGenFencesNV	qgl.GenFencesNV
#define qglDeleteFencesNV	qgl.DeleteFencesNV
#define qglSetFenceNV	qgl.SetFenceNV
#define qglFinishFenceNV	qgl.FinishFenceNV

#ifdef _WIN32
#define qwglSwapIntervalEXT	qgl.wglSwapIntervalEXT
#define qglVertexArrayRangeNV	qgl.glVertexArrayRangeNV
#define qglFlushVertexArrayRangeNV	qgl.glFlushVertexArrayRangeNV
#define qglAllocateMemoryNV	qgl.glAllocateMemoryNV
#define qglFreeMemoryNV	qgl.glFreeMemoryNV
#endif

#ifdef __linux__
#define qglVertexArrayRangeNV	qgl.glVertexArrayRangeNV
#define qglFlushVertexArrayRangeNV	qgl.glFlushVertexArrayRangeNV
#define qglAllocateMemoryNV	qgl.glAllocateMemoryNV
#define qglFreeMemoryNV	qgl.glFreeMemoryNV
#endif


#define QGL_SGIS_MULTITEXTURE	(1 << 0)
#define QGL_ARB_MULTITEXTURE	(1 << 1)
#define QGL_S3_S3TC	(1 << 2)
#define QGL_ARB_TEXTURE_COMPRESSION	(1 << 3)
#define QGL_EXT_COMPILED_VERTEX_ARRAY	(1 << 4)
#define QGL_NV_FENCE	(1 << 5)

#ifdef _WIN32
#define QWGL_EXT_SWAP_CONTROL	(1 << 6)
#define QGL_NV_VERTEX_ARRAY_RANGE	(1 << 7)
#endif

#ifdef __linux__
#define QGL_NV_VERTEX_ARRAY_RANGE	(1 << 6)
#endif

