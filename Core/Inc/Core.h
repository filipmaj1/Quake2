/*=============================================================================

	Main core header

=============================================================================*/


#ifndef __CORE_H__
#define __CORE_H__


//!! add checking of CORE_VERSION (in most cases, executable will not be loaded, but ...)
#define CORE_VERSION	0


#ifndef CORE_API
#	define CORE_API DLL_IMPORT
#endif


// C standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

// Build options
#include "Build.h"

// Platform/compiler-specific code
#ifdef _MSC_VER
#	include "VcWin32.h"
#else
#	error Unknown compiler
#endif


// guard/unguard/unguardf[Slow] macros
#if !DO_GUARD
#	undef guard
#	undef unguard
#	undef unguardf
#	define guard(func)
#	define unguard
#	define unguardf(msg)
#endif

#if DO_GUARD_SLOW
#	define guardSlow(func)		guard(func)
#	define unguardSlow			unguard
#	define unguardfSlow(msg)	unguardf(msg)
#else
#	define guardSlow(func)		{
#	define unguardSlow			}
#	define unguardfSlow(msg)	}
#endif


#ifndef NULL
#	define NULL ((void *)0)
#endif



/*----------------------------------------------------------------------------
	Forward declarations
----------------------------------------------------------------------------*/

class CMemoryChain;


/*-----------------------------------------------------------------------------
	Structures
-----------------------------------------------------------------------------*/

//??


/*-----------------------------------------------------------------------------
	Text output system
-----------------------------------------------------------------------------*/

// Output device
class CORE_API COutputDevice
{
public:
	virtual void Write (const char *str) = 0;
	void Printf (const char *fmt, ...);
};


//?? #define appPrintf	GLog->Printf
void Com_Printf(const char *fmt, ...);
#define appPrintf Com_Printf

//?? reimplement
CORE_API void appWPrintf (const char *fmt, ...);


/*-----------------------------------------------------------------------------
	Error management
-----------------------------------------------------------------------------*/

class CORE_API CErrorHandler
{
public:
	bool	swError;			// true when error was thrown by appError() call; will not dump CPU context
	bool	fatalError;			// when false, app can try to recover from error (and swError will be true)
	bool	wasError;			// used for error history formatting
	char	history[4096];
	char	message[128];		//?? for "server crashed ..." message
	void	Reset ();
};

CORE_API NORETURN void appFatalError (const char *fmt, ...);
#define appError	appFatalError
CORE_API NORETURN void appNonFatalError (const char *fmt, ...);


CORE_API void appUnwindPrefix (const char *fmt);		// not vararg (will display function name for unguardf only)
CORE_API NORETURN void appUnwindThrow (const char *fmt, ...);


/*-----------------------------------------------------------------------------
	Process control
-----------------------------------------------------------------------------*/

// Initialize core
CORE_API void appInit (/*?? const char *_cmdLine, COutputDevice *_log */);
CORE_API void appExit (void);


/*-----------------------------------------------------------------------------
	Normal includes
-----------------------------------------------------------------------------*/

#include "Strings.h"
#include "DbgSymbols.h"						// MAY have some macros, so go as first as possible
#include "Macro.h"
#include "MemoryMgr.h"
//!! #include "Cvar.h"
#include "Commands.h"
//!! #include "FileSystem.h"

/*-----------------------------------------------------------------------------
	Global variables
-----------------------------------------------------------------------------*/

//??CORE_API extern COutputDevice	*GLog;
//??CORE_API extern COutputDevice	*GNull;
CORE_API extern CErrorHandler	GErr;
//??CORE_API extern char GVersion[512];

/*-----------------------------------------------------------------------------
	Variables for current package
-----------------------------------------------------------------------------*/

//??
#if 0
extern "C"
{
	//!! this vars can be non-DLL_EXPORT for main package
	DLL_EXPORT extern const char	GPackage[];			// const name of current package
	DLL_EXPORT extern const char	PkgVersion[];
	extern HINSTANCE hInstance;
}
#endif

#endif