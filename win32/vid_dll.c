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
// Main windowed and fullscreen graphics interface module. This module
// is used for both the software and OpenGL rendering versions of the
// Quake refresh engine.
#include <assert.h>
#include <float.h>

#include "../qcommon/qcommon.h"

#include "../client/client.h"
#include "winquake.h"
#include "resource.h"

// Structure containing functions exported from refresh DLL
refExport_t	re;

// Imports from main engine for renderer DLL
#include "../client/ref_impl.h"

cvar_t *win_noalttab;
cvar_t *win_priorityBoost;

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)  // message that will be supported by the OS
#endif

static UINT MSH_MOUSEWHEEL;

// Console variables that we need to access from this module
cvar_t	*r_gamma;
cvar_t	*vid_ref;			// Name of Refresh DLL loaded
cvar_t	*vid_xpos;			// X coordinate of window position
cvar_t	*vid_ypos;			// Y coordinate of window position
cvar_t	*r_fullscreen;

// Global variables used internally by this module
viddef_t viddef;					// global video state; used by other modules
static HINSTANCE refLibrary;		// Handle to refresh DLL
static qboolean refActive = false;

HWND	cl_hwnd;			// Main window handle for life of program

LONG WINAPI MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static qboolean s_alttab_disabled;

extern unsigned sys_msg_time;

/*
** WIN32 helper functions
*/
extern qboolean s_win95;

static void WIN_DisableAltTab (void)
{
	if (s_alttab_disabled)
		return;

	if (s_win95)
	{
		BOOL old;

		SystemParametersInfo (SPI_SCREENSAVERRUNNING, 1, &old, 0);
	}
	else
	{
		RegisterHotKey (0, 0, MOD_ALT, VK_TAB);
		RegisterHotKey (0, 1, MOD_ALT, VK_RETURN);
	}
	s_alttab_disabled = true;
}

static void WIN_EnableAltTab (void)
{
	if (s_alttab_disabled)
	{
		if (s_win95)
		{
			BOOL old;

			SystemParametersInfo (SPI_SCREENSAVERRUNNING, 0, &old, 0);
		}
		else
		{
			UnregisterHotKey (0, 0);
			UnregisterHotKey (0, 1);
		}
		s_alttab_disabled = false;
	}
}

static void WIN_HighPriority (qboolean enable)
{
	HANDLE	hProcess;

	hProcess = GetCurrentProcess ();
	SetPriorityClass (hProcess, enable ? HIGH_PRIORITY_CLASS : NORMAL_PRIORITY_CLASS);
}


//==========================================================================

static byte scantokey[128] =
{
//	0		1		2		3		4		5		6		7
//	8		9		A		B		C		D		E		F			//
	0,		27,		'1',	'2',	'3',	'4',	'5',	'6',
	'7',	'8',	'9',	'0',	'-',	'=',	K_BACKSPACE, 9,		// 0
	'q',	'w',	'e',	'r',	't',	'y',	'u',	'i',
	'o',	'p',	'[',	']',	13 ,	K_CTRL,	'a',	's',		// 1
	'd',	'f',	'g',	'h',	'j',	'k',	'l',	';',
	'\'',	'`',	K_SHIFT,'\\',	'z',	'x',	'c',	'v',		// 2
	'b',	'n',	'm',	',',	'.',	'/',	K_SHIFT,'*',
	K_ALT,	' ',	0,		K_F1,	K_F2,	K_F3,	K_F4,	K_F5,		// 3
	K_F6,	K_F7,	K_F8,	K_F9,	K_F10,	K_PAUSE,K_UNK46,K_HOME,
	K_UPARROW,K_PGUP,K_KP_MINUS,K_LEFTARROW,K_KP_5,K_RIGHTARROW,K_KP_PLUS,K_END, //4
	K_DOWNARROW,K_PGDN,K_INS,K_DEL, 0,		0,		0,		K_F11,
	K_F12,	K_UNK59,K_UNK5A,K_UNK5B,K_UNK5C,K_UNK5D,K_UNK5E,K_UNK5F,	// 5
	0,		0,		0,		0,		0,		0,		0,		0,
	0,		0,		0,		0,		0,		0,		0,		0,			// 6
	0,		0,		0,		0,		0,		0,		0,		0,
	0,		0,		0,		0,		0,		0,		0,		0			// 7
};

/*
=======
MapKey

Map from windows to quake keynums
=======
*/
static int MapKey (int key)
{
	int result;
	int modified = (key >> 16) & 255;
	qboolean is_extended = false;

	if (modified > 127)
		return 0;

	if (key & (1 << 24))
		is_extended = true;

	result = scantokey[modified];

	if (!is_extended)
	{
		switch (result)
		{
		case K_HOME:
			return K_KP_HOME;
		case K_UPARROW:
			return K_KP_UPARROW;
		case K_PGUP:
			return K_KP_PGUP;
		case K_LEFTARROW:
			return K_KP_LEFTARROW;
		case K_RIGHTARROW:
			return K_KP_RIGHTARROW;
		case K_END:
			return K_KP_END;
		case K_DOWNARROW:
			return K_KP_DOWNARROW;
		case K_PGDN:
			return K_KP_PGDN;
		case K_INS:
			return K_KP_INS;
		case K_DEL:
			return K_KP_DEL;
		default:
			return result;
		}
	}
	else
	{
		switch (result)
		{
		case 0x0D:
			return K_KP_ENTER;
		case 0x2F:
			return K_KP_SLASH;
		case 0xAF:
			return K_KP_PLUS;
		}
		return result;
	}
}

void AppActivate (BOOL fActive, BOOL minimize)
{
	Minimized = minimize;

	Key_ClearStates ();

	// we don't want to act like we're active if we're minimized
	if (fActive && !Minimized)
		ActiveApp = true;
	else
		ActiveApp = false;

	// minimize/restore mouse-capture on demand
	if (!ActiveApp)
	{
		IN_Activate (false);
		CDAudio_Activate (false);
		S_Activate (false);

		if (win_noalttab->integer)
			WIN_EnableAltTab ();
		if (win_priorityBoost->integer)
			WIN_HighPriority (false);
	}
	else
	{
		IN_Activate (true);
		CDAudio_Activate (true);
		S_Activate (true);
		if (win_noalttab->integer)
			WIN_DisableAltTab ();
		if (win_priorityBoost->integer)
			WIN_HighPriority (true);
	}
}

/*
====================
MainWndProc

main window procedure
====================
*/
static LONG WINAPI MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LONG	lRet = 0;

	if (uMsg == MSH_MOUSEWHEEL)
	{
		if ((int)wParam > 0)
		{
			Key_Event (K_MWHEELUP, true, sys_msg_time);
			Key_Event (K_MWHEELUP, false, sys_msg_time);
		}
		else
		{
			Key_Event (K_MWHEELDOWN, true, sys_msg_time);
			Key_Event (K_MWHEELDOWN, false, sys_msg_time);
		}
		return DefWindowProc (hWnd, uMsg, wParam, lParam);
	}

	switch (uMsg)
	{
	case WM_MOUSEWHEEL:
		// this chunk of code theoretically only works under NT4 and Win98
		// since this message doesn't exist under Win95
		if ((short) HIWORD(wParam) > 0)
		{
			Key_Event (K_MWHEELUP, true, sys_msg_time);
			Key_Event (K_MWHEELUP, false, sys_msg_time);
		}
		else
		{
			Key_Event (K_MWHEELDOWN, true, sys_msg_time);
			Key_Event (K_MWHEELDOWN, false, sys_msg_time);
		}
		break;

	case WM_HOTKEY:
		return 0;

	case WM_CREATE:
		cl_hwnd = hWnd;

		MSH_MOUSEWHEEL = RegisterWindowMessage ("MSWHEEL_ROLLMSG");
		break;

	case WM_PAINT:
		SCR_DirtyScreen ();	// force entire screen to update next frame
		break;

	case WM_DESTROY:
		// let sound and input know about this?
		cl_hwnd = NULL;
		break;

	case WM_ACTIVATE:
		{
			int	fActive, fMinimized;

			// KJB: Watch this for problems in fullscreen modes with Alt-tabbing.
			fActive = LOWORD(wParam);
			fMinimized = (BOOL) HIWORD(wParam);

			AppActivate (fActive != WA_INACTIVE, fMinimized);

			if (refActive)
				re.AppActivate (!(fActive == WA_INACTIVE));
		}
		break;

	case WM_MOVE:
		{
			int		xPos, yPos;
			RECT r;
			int		style;

			if (!r_fullscreen->integer)
			{
				xPos = (short) LOWORD(lParam);    // horizontal position
				yPos = (short) HIWORD(lParam);    // vertical position

				r.left   = 0;
				r.top    = 0;
				r.right  = 1;
				r.bottom = 1;

				style = GetWindowLong (hWnd, GWL_STYLE);
				AdjustWindowRect (&r, style, FALSE);

				Cvar_SetInteger ("vid_xpos", xPos + r.left);
				Cvar_SetInteger ("vid_ypos", yPos + r.top);
				vid_xpos->modified = false;
				vid_ypos->modified = false;
				if (ActiveApp)
					IN_Activate (true);
			}
		}
		break;

	// this is complicated because Win32 seems to pack multiple mouse events into
	// one update sometimes, so we always check all states and look for events
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
		{
			int		temp;

			temp = 0;

			if (wParam & MK_LBUTTON)
				temp |= 1;

			if (wParam & MK_RBUTTON)
				temp |= 2;

			if (wParam & MK_MBUTTON)
				temp |= 4;

			IN_MouseEvent (temp);
		}
		break;

	case WM_SYSCOMMAND:
		if (wParam == SC_SCREENSAVE)
			return 0;
		if (wParam == SC_KEYMENU)	// disable activating window menu with keyboard
			return 0;
		break;
	case WM_SYSKEYDOWN:
		if (wParam == 13) 			// Alt+Enter
		{
			if (r_fullscreen) Cvar_SetInteger ("r_fullscreen", !r_fullscreen->integer);
			return 0;
		}
		// fall through
	case WM_KEYDOWN:
		{
			int		k;

			k = MapKey (lParam);
			if (!k) Com_Printf ("Unknown key: 0x%X\n", (lParam >> 16) & 255);
			Key_Event (k, true, sys_msg_time);
		}
		break;

	case WM_SYSKEYUP:
	case WM_KEYUP:
		Key_Event (MapKey (lParam), false, sys_msg_time);
		break;

	case WM_CLOSE:
		Com_Quit ();
		break;		// should not happens (?)

	case MM_MCINOTIFY:
		{
			LONG CDAudio_MessageHandler (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

			lRet = CDAudio_MessageHandler (hWnd, uMsg, wParam, lParam);
		}
		break;
	}

	return DefWindowProc (hWnd, uMsg, wParam, lParam);
}


/*
 * Vid_NewWindow
 */
static void Vid_NewWindow (int width, int height)
{
	viddef.width  = width;
	viddef.height = height;

	cl.force_refdef = true;		// can't use a paused refdef
}

/*
 * Vid_CreateWindow
 */

static HWND mainHwnd;

void *Vid_CreateWindow (int width, int height, qboolean fullscreen)
{
	WNDCLASS wc;
	RECT	r;
	int		stylebits;
	int		x, y, w, h;
	int		exstyle;

	if (fullscreen)
	{
		exstyle = WS_EX_TOPMOST;
		stylebits = WS_POPUP|WS_SYSMENU;
	}
	else
	{
		exstyle = 0;
		stylebits = WS_SYSMENU|WS_CAPTION;
	}

	// if size=0 -- invisible (fake) window
//	if (width || height)		// if enable this, window will be created without taskbar button!
//		stylebits |= WS_VISIBLE;

	r.left = 0;
	r.top = 0;
	r.right  = width;
	r.bottom = height;

	AdjustWindowRect (&r, stylebits, FALSE);

	w = r.right - r.left;
	h = r.bottom - r.top;

	if (fullscreen)
	{
		x = 0;
		y = 0;
	}
	else
	{
		x = Cvar_VariableInt ("vid_xpos");
		y = Cvar_VariableInt ("vid_ypos");
	}

	if (mainHwnd)
	{
		SetWindowLong (mainHwnd, GWL_STYLE, stylebits);
		SetWindowLong (mainHwnd, GWL_EXSTYLE, exstyle);
		ShowWindow (mainHwnd, SW_SHOW);
		SetWindowPos (mainHwnd, 0, x, y, w, h, SWP_NOZORDER);

		UpdateWindow (mainHwnd);
		SetForegroundWindow (mainHwnd);
		SetFocus (mainHwnd);

		Vid_NewWindow (width, height);
		return mainHwnd;
	}

	// Register the frame class
	wc.style			= 0;
	wc.lpfnWndProc		= MainWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= global_hInstance;
	wc.hIcon			= LoadIcon (global_hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor			= LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground	= (void *) COLOR_GRAYTEXT;
	wc.lpszMenuName 	= NULL;
	wc.lpszClassName	= APPNAME;

	if (!RegisterClass (&wc))
		Com_Error (ERR_FATAL, "Vid_CreateWindow: couldn't register window class");

	mainHwnd = CreateWindowEx (
		exstyle,
		APPNAME, APPNAME,
		stylebits,
		x, y, w, h,
		NULL, NULL,
		global_hInstance, NULL);

	if (!mainHwnd) Com_Error (ERR_FATAL, "Vid_CreateWindow: couldn't create window");

	if (width || height)
		ShowWindow (mainHwnd, SW_SHOW);
	UpdateWindow (mainHwnd);

	SetForegroundWindow (mainHwnd);
	SetFocus (mainHwnd);

	// let the sound and input subsystems know about the new window
	Vid_NewWindow (width, height);

	return mainHwnd;
}

/*
 * Vid_DestroyWindow
 */

void Vid_DestroyWindow (qboolean force)
{
	if (!force)	//?? add cvar "win_singleWindow"
	{
//		ShowWindow (mainHwnd, SW_HIDE);
		return;
	}

	Com_DPrintf ("...destroying window\n");
	if (mainHwnd)
	{
		ShowWindow (mainHwnd, SW_HIDE);		// as Q3 does...
		DestroyWindow (mainHwnd);
		mainHwnd = 0;
	}

	UnregisterClass (APPNAME, global_hInstance);
}


/*
============
Vid_Restart_f

Console command to re-start the video mode and refresh DLL. We do this
simply by setting the modified flag for the vid_ref variable, which will
cause the entire video mode and refresh DLL to be reset on the next frame.
============
*/
static void Vid_Restart_f (void)
{
	vid_ref->modified = true;
}

static void Vid_Front_f( void )
{
	SetWindowLong (cl_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);
	SetForegroundWindow (cl_hwnd);
}


/*------------- Vid_GetModeInfo -------------*/

typedef struct
{
	int		width, height;
} vidMode_t;

static vidMode_t vid_modes[] =
{
	{320,	240},
	{400,	300},
	{512,	384},
	{640,	480},
	{800,	600},
	{960,	720},
	{1024,	768},
	{1152,	864},
	{1280,	960},
	{1600,	1200},
	{2048,	1536}
};
#define VID_NUM_MODES (sizeof(vid_modes) / sizeof(vid_modes[0]))


qboolean Vid_GetModeInfo (int *width, int *height, int mode)
{
	if (mode < 0 || mode >= VID_NUM_MODES)
		return false;

	*width  = vid_modes[mode].width;
	*height = vid_modes[mode].height;

	return true;
}


/*
** Vid_UpdateWindowPosAndSize
*/
void Vid_UpdateWindowPosAndSize (int x, int y)
{
	RECT	r;
	int		style;
	int		w, h;

	r.left   = 0;
	r.top    = 0;
	r.right  = viddef.width;
	r.bottom = viddef.height;

	style = GetWindowLong (cl_hwnd, GWL_STYLE);
	AdjustWindowRect (&r, style, FALSE);

	w = r.right - r.left;
	h = r.bottom - r.top;

	MoveWindow (cl_hwnd, vid_xpos->integer, vid_ypos->integer, w, h, TRUE);
}

void Vid_FreeReflib (void)
{
	refActive  = false;
	if (!refLibrary) return;	// statically linked?

	if (!FreeLibrary (refLibrary))
		Com_Error (ERR_FATAL, "Reflib FreeLibrary() failed");
	memset (&re, 0, sizeof(re));
	refLibrary = NULL;
}

/*
==============
Vid_LoadRefresh
==============
*/

/*-------- Dummy functions for console-only mode ---------*/

static void	D_RenderFrame (refdef_t *fd) {}
static void	D_BeginRegistration (char *map) {}
static struct model_s *D_RegisterModel (char *name) { return NULL; }
static struct image_s *D_RegisterSkin (char *name) { return NULL; }
static struct image_s *D_FindPic (char *name) { return NULL; }
static void D_SetSky (char *name, float rotate, vec3_t axis) {}
static void	D_EndRegistration (void) {}

static void	D_Draw_GetPicSize (int *w, int *h, char *pic)
{
	if (w) *w = 0;
	if (h) *h = 0;
}

static void D_Screenshot (int flags, char *name)
{
	Com_WPrintf ("Screenshots are unsupported by this renderer\n");
}

static void	D_Draw_PicColor (int x, int y, char *name, int color) {}
static void	D_Draw_StretchPic (int x, int y, int w, int h, char *pic) {}
static void	D_Draw_CharColor (int x, int y, int c, int color) {}
static void	D_Draw_TileClear (int x, int y, int w, int h, char *name) {}
static void	D_Draw_Fill (int x, int y, int w, int h, int c) {}
static void D_Draw_Fill2 (int x, int y, int w, int h, float r, float g, float b, float a) {}
static void	D_DrawTextPos (int x, int y, char *text, float r, float g, float b) {}
static void	D_DrawTextSide (char *text, float r, float g, float b) {}
static void	D_Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data) {}
static void	D_SetPalette (const unsigned char *palette) {}
static float D_GetClientLight (void) { return 0; }		// normal value is 150


#ifdef REF_HARD_LINKED
extern refExport_t GL_GetRefAPI (refImport_t rimp);		// ref_gl
extern refExport_t GetRefAPI (refImport_t rimp);		// ref_soft
#endif

static qboolean Vid_LoadRefresh (char *name)
{
	GetRefAPI_t	LibGetRefAPI;
	char	dllName[MAX_OSPATH];

	if (refActive)
	{
		re.Shutdown ();
		Vid_FreeReflib ();
	}

	Com_sprintf (dllName, sizeof(dllName), "ref_%s.dll", name);
	Com_Printf ("------- Loading %s -------\n", name);

#ifdef REF_HARD_LINKED
	refLibrary = NULL;

	if (!strcmp (name, "gl"))
		LibGetRefAPI = GL_GetRefAPI;
	else if (!strcmp (name, "soft"))
		LibGetRefAPI = GetRefAPI;
	else
#endif
	{
		if (!(refLibrary = LoadLibrary (dllName)))
		{
			Com_WPrintf ("LoadLibrary(\"%s\") failed\n", dllName);
			return false;
		}
		if (!(LibGetRefAPI = (void *) GetProcAddress (refLibrary, "GetRefAPI")))
		{
			Com_WPrintf ("GetProcAddress() failed on %s\n", dllName);
			Vid_FreeReflib ();
			return false;
		}
	}

	re = LibGetRefAPI (ri);

	if (re.struc_size != sizeof(refExport_t) || re.api_version != API_VERSION)
	{
		Com_WPrintf ("%s has incompatible API_VERSION\n", dllName);
		Vid_FreeReflib ();
		return false;
	}

	if (*re.flags & REF_CONSOLE_ONLY)
	{
		re.RenderFrame =	D_RenderFrame;
		re.Screenshot =		D_Screenshot;
		re.BeginRegistration = D_BeginRegistration;
		re.RegisterModel =	D_RegisterModel;
		re.RegisterSkin =	D_RegisterSkin;
		re.RegisterPic =	D_FindPic;
		re.SetSky =			D_SetSky;
		re.EndRegistration = D_EndRegistration;

		re.DrawGetPicSize =	D_Draw_GetPicSize;
		re.DrawPicColor =	D_Draw_PicColor;
		re.DrawStretchPic =	D_Draw_StretchPic;
		re.DrawCharColor =	D_Draw_CharColor;
		re.DrawTileClear =	D_Draw_TileClear;
		re.DrawFill =		D_Draw_Fill;
		re.DrawFill2 =		D_Draw_Fill2;

		re.DrawStretchRaw =	D_Draw_StretchRaw;
		re.CinematicSetPalette = D_SetPalette;

		re.DrawTextPos =	D_DrawTextPos;
		re.DrawTextLeft =	D_DrawTextSide;
		re.DrawTextRight =	D_DrawTextSide;

		re.GetClientLight = D_GetClientLight;
	}

	if (re.Init () == -1)
	{
		re.Shutdown ();
		Vid_FreeReflib ();
		return false;
	}

	Com_Printf ("------------------------------------\n");
	refActive = true;

	return true;
}

/*
============
Vid_CheckChanges

This function gets called once just before drawing each frame, and it's sole purpose in life
is to check to see if any of the video mode parameters have changed, and if they have to
update the rendering DLL and/or video mode to match.
============
*/

static char lastRenderer[MAX_QPATH];

void Vid_CheckChanges (void)
{
	if (win_noalttab->modified)
	{
		if (win_noalttab->integer)
			WIN_DisableAltTab ();
		else
			WIN_EnableAltTab ();

		win_noalttab->modified = false;
	}

	if (win_priorityBoost->modified)
	{
		WIN_HighPriority (win_priorityBoost->integer);
		win_priorityBoost->modified = false;
	}

	if (vid_ref->modified)
	{
		qboolean loaded;

		// refresh has changed
		cl.force_refdef = true;		// can't use a paused refdef
		S_StopAllSounds ();

		r_fullscreen->modified = true;
		cl.refresh_prepped = false;
		cls.disable_screen = true;

		loaded = false;
		if (Vid_LoadRefresh (vid_ref->string))
			loaded = true;
/*		else
		{
			// drop the console if we fail to load a refresh
			if (cls.key_dest != key_console) Con_ToggleConsole_f ();
		} */
		if (!loaded && lastRenderer)
		{
			if (Vid_LoadRefresh (lastRenderer))
			{
				Cvar_Set ("vid_ref", lastRenderer);
				loaded = true;
			}
		}
		if (!loaded)
		{
			if (!strcmp (vid_ref->string, "soft") || !Vid_LoadRefresh ("soft"))
				Com_Error (ERR_FATAL, "Couldn't fall back to software refresh");
			Cvar_Set ("vid_ref", "soft");
		}
		strcpy (lastRenderer, vid_ref->string);
		vid_ref->modified = false;
		cls.disable_screen = false;
	}

	// update our window position
	if (vid_xpos->modified || vid_ypos->modified)
	{
		if (!r_fullscreen->integer)
			Vid_UpdateWindowPosAndSize (vid_xpos->integer, vid_ypos->integer);

		vid_xpos->modified = false;
		vid_ypos->modified = false;
	}
}

/*
============
Vid_Init
============
*/
void Vid_Init (void)
{
CVAR_BEGIN(vars)
	CVAR_VAR(vid_ref, soft, CVAR_ARCHIVE),
	CVAR_VAR(vid_xpos, 0, CVAR_ARCHIVE),
	CVAR_VAR(vid_ypos, 0, CVAR_ARCHIVE),
	CVAR_VAR(r_fullscreen, 1, CVAR_ARCHIVE),
	CVAR_VAR(r_gamma, 1, CVAR_ARCHIVE),
	CVAR_VAR(win_noalttab, 0, CVAR_ARCHIVE),
	CVAR_VAR(win_priorityBoost, 0, CVAR_ARCHIVE)
CVAR_END

	CVAR_GET_VARS(vars);

	// Add some console commands that we want to handle
	Cmd_AddCommand ("vid_restart", Vid_Restart_f);
	Cmd_AddCommand ("vid_front", Vid_Front_f);

#if 0
	// this is a gross hack but necessary to clamp the mode for 3Dfx
	{
		cvar_t *gl_driver = Cvar_Get ("gl_driver", "opengl32", 0);
		cvar_t *gl_mode = Cvar_Get ("gl_mode", "3", 0);

		if (stricmp (gl_driver->string, "3dfxgl") == 0)
		{
			Cvar_SetInteger ("gl_mode", 3);
			viddef.width  = 640;
			viddef.height = 480;
		}
	}
#endif
	// Disable the 3Dfx splash screen
	putenv ("FX_GLIDE_NO_SPLASH=0");

	// Create invisible (fake) window to capture Win32 focus
	Vid_CreateWindow (0, 0, false);

	/* Start the graphics mode and load refresh DLL */
	Vid_CheckChanges ();
}

/*
============
Vid_Shutdown
============
*/
void Vid_Shutdown (void)
{
	if (refActive)
	{
		/* signal for ref_xx.dll to completely shutdown (Vid_Shutdown(); it is called after
		 * CL_WriteConfiguration(), so we can safely change vid_ref value
		 */
		Cvar_Set ("vid_ref", "");
		re.Shutdown ();
		Vid_DestroyWindow (true);
		Vid_FreeReflib ();
	}
}
