#ifdef _WIN32
#include <windows.h> 
#else
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <malloc.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "sys_ded.h"
#include "conproc.h"
#include "dedicated.h"
#include "exefuncs.h"
#include "crc.h"

#include "dll_state.h"
#include "enginecallback.h"

#if defined( _WIN32 )
#include "../utils/procinfo/procinfo.h"
#endif

#ifdef _WIN32
static const char *g_pszengine = "sw.dll";
#else
static const char *g_pszengine = "engine_i386.so";
#endif

static char g_szEXEName[256];

#define MINIMUM_WIN_MEMORY		0x0c00000 
#define MAXIMUM_WIN_MEMORY		0x2000000 // max 32 mb
#define FIFTEEN_MEGS ( 15 * 1024 * 1024 ) 

#ifdef _WIN32
static	HANDLE	hinput;
static  HANDLE	houtput;
#endif

// System Memory & Size
static unsigned char	*gpMemBase = NULL;
static int				giMemSize = 0x1800000;  // 24 Mb Linux heapsize

exefuncs_t ef;

static char	console_text[256];
static int	console_textlen;

static long hDLLThirdParty = 0L;

char			*gpszCmdLine = NULL;

void Sys_Sleep( int msec )
{
#ifdef _WIN32
	Sleep( msec );
#else
    usleep(msec * 1000);
#endif
}

void *Sys_GetProcAddress( long library, const char *name )
{
#ifdef _WIN32
	return ( void * )GetProcAddress( (HMODULE)library, name );
#else // LINUX
	return dlsym( (void *)library, name );
#endif
}

long Sys_LoadLibrary( char *lib )
{
	void *hDll = NULL;

#ifdef _WIN32
	hDll = ::LoadLibrary( lib );
#else
    char    cwd[1024];
    char    absolute_lib[1024];
    
    if (!getcwd(cwd, sizeof(cwd)))
        Sys_ErrorMessage(1, "Sys_LoadLibrary: Couldn't determine current directory.");
        
    if (cwd[strlen(cwd)-1] == '/')
        cwd[strlen(cwd)-1] = 0;
        
	_snprintf(absolute_lib, sizeof(absolute_lib), "%s/%s", cwd, lib);
    
    hDll = dlopen( absolute_lib, RTLD_NOW );
    if ( !hDll )
    {
        Sys_ErrorMessage( 1, dlerror() );
    }   
#endif
	return (long)hDll;
}

void Sys_FreeLibrary( long library )
{
	if ( !library )
		return;

#ifdef _WIN32
	::FreeLibrary( (HMODULE)library );
#else
	dlclose( (void *)library );
#endif
}

/*
==============
Sys_ErrorMessage

Engine is erroring out, display error in message box
==============
*/
void Sys_ErrorMessage( int level, const char *msg )
{
#ifdef _WIN32
	MessageBox( NULL, msg, "Half-Life", MB_OK );
	PostQuitMessage(0);	
#else
	printf( "%s\n", msg );
	exit( -1 );
#endif
}

#ifdef _WIN32
/*
==============
UpdateStatus

Update status line at top of console if engine is running
==============
*/
void UpdateStatus( int force )
{
	static double tLast = 0.0;
	double	tCurrent;
	char	szPrompt[256];
	int		n, spec, nMax;
	char	szMap[32];
	float	fps;

	if ( !engineapi.Host_GetHostInfo )
		return;

	tCurrent = (float)( timeGetTime() / 1000.0f );

	engineapi.Host_GetHostInfo( &fps, &n, &spec, &nMax, szMap );

	if ( !force )
	{
		if ( ( tCurrent - tLast ) < 0.5f )
			return;
	}

	tLast = tCurrent;

	_snprintf(szPrompt, sizeof(szPrompt), "%.1f fps %2i(%2i spec)/%2i on %16s", (float)fps, n, spec, nMax, szMap);

	WriteStatusText( szPrompt );
}
#endif

/*
================
Sys_ConsoleOutput

Print text to the dedicated console
================
*/
void Sys_ConsoleOutput (char *string)
{
#ifdef _WIN32
	unsigned long dummy;
	char	text[256];

	if (console_textlen)
	{
		text[0] = '\r';
		memset(&text[1], ' ', console_textlen);
		text[console_textlen+1] = '\r';
		text[console_textlen+2] = 0;
		WriteFile(houtput, text, console_textlen+2, &dummy, NULL);
	}

	WriteFile(houtput, string, strlen(string), &dummy, NULL);

	if (console_textlen)
	{
		WriteFile(houtput, console_text, console_textlen, &dummy, NULL);
	}
	UpdateStatus( 1 /* force */ );
#else
	printf( string );
	fflush(stdout);
#endif
}

/*
==============
Sys_Printf

Engine is printing to console
==============
*/
void Sys_Printf(char *fmt, ...)
{
	// Dump text to debugging console.
	va_list argptr;
	char szText[1024];

	va_start (argptr, fmt);
	vsprintf (szText, fmt, argptr);
	va_end (argptr);

	// Get Current text and append it.
	Sys_ConsoleOutput( szText );
}

/*
==============
Load3rdParty

Load support for third party .dlls ( gamehost )
==============
*/
void Load3rdParty( void )
{
	// Only do this if the server operator wants the support.
	// ( In case of malicious code, too )
	if ( CheckParm( "-usegh" ) )   
	{
		hDLLThirdParty = Sys_LoadLibrary( "ghostinj.dll" );
	}
}

/*
==============
EF_VID_ForceUnlockedAndReturnState

Dummy funcion called by engine
==============
*/
int  EF_VID_ForceUnlockedAndReturnState(void)
{
	return 0;
}

/*
==============
EF_VID_ForceLockState

Dummy funcion called by engine
==============
*/
void EF_VID_ForceLockState(int)
{
}

/*
==============
CheckParm

Search for psz in command line to .exe, if **ppszValue is set, then the pointer is
 directed at the NEXT argument in the command line
==============
*/
char *CheckParm(const char *psz, char **ppszValue)
{
	int i;
	static char sz[128];
	char *pret;

	if (!gpszCmdLine)
		return NULL;

	pret = strstr( gpszCmdLine, psz );

	// should we return a pointer to the value?
	if (pret && ppszValue)
	{
		char *p1 = pret;
		*ppszValue = NULL;

		while ( *p1 && (*p1 != 32))
			p1++;

		if (p1 != 0)
		{
			char *p2 = ++p1;

			for ( i = 0; i < 128; i++ )
			{
				if ( !*p2 || (*p2 == 32))
					break;
				sz[i] = *p2++;
			}

			sz[i] = 0;
			*ppszValue = &sz[0];		
		}	
	}

	return pret;
}

/*
==============
InitInstance

==============
*/
int InitInstance( void )
{
	Load3rdParty();

	Eng_SetState( DLL_INACTIVE );

	memset( &ef, 0, sizeof( ef ) );
	
	// Function pointers used by dedicated server
	ef.Sys_Printf							= Sys_Printf;
	ef.ErrorMessage							= Sys_ErrorMessage;

	ef.VID_ForceLockState					= EF_VID_ForceLockState;
	ef.VID_ForceUnlockedAndReturnState		= EF_VID_ForceUnlockedAndReturnState;

#ifdef _WIN32
	// Data
	ef.fMMX									= PROC_IsMMX();
	ef.iCPUMhz								= PROC_GetSpeed();	// in MHz
#endif

	return 1;
}

/*
================
Sys_ConsoleInput

================
*/
#ifdef _WIN32
char *Sys_ConsoleInput (void)
{
	INPUT_RECORD	recs[1024];
	unsigned long	dummy;
	int				ch;
	unsigned long	numread, numevents;

	while ( 1 )
	{
		if (!GetNumberOfConsoleInputEvents (hinput, &numevents))
		{
			exit( -1 );
		}

		if (numevents <= 0)
			break;

		if ( !ReadConsoleInput(hinput, recs, 1, &numread) )
		{
			exit( -1 );
		}

		if (numread != 1)
		{
			exit( -1 );
		}

		if ( recs[0].EventType == KEY_EVENT )
		{
			if ( !recs[0].Event.KeyEvent.bKeyDown )
			{
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;
				switch (ch)
				{
					case '\r':
						WriteFile(houtput, "\r\n", 2, &dummy, NULL);	
						if (console_textlen)
						{
							console_text[console_textlen] = 0;
							console_textlen = 0;
							return console_text;
						}
						break;

					case '\b':
						if (console_textlen)
						{
							console_textlen--;
							WriteFile(houtput, "\b \b", 3, &dummy, NULL);	
						}
						break;

					default:
						if (ch >= ' ')
						{
							if (console_textlen < sizeof(console_text)-2)
							{
								WriteFile(houtput, &ch, 1, &dummy, NULL);	
								console_text[console_textlen] = ch;
								console_textlen++;
							}
						}

						break;

				}
			}
		}
	}

	return NULL;
}
#else
char *Sys_ConsoleInput(void)
{
	char ch;
    int     len;
	fd_set	fdset;
    struct timeval timeout;
	char szInput[256];
	char iInput = 0;

	FD_ZERO(&fdset);
	FD_SET(0, &fdset); // stdin
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	if (select (1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(0, &fdset))
		return NULL;

	while (read(0,&ch,1))
	{
		if (iInput >= 255)
			continue;

		if (ch == 10) 
		{
			char *pszret = NULL;

			//Null terminate string and return if we have anything
			szInput[iInput] = 0;

			if (iInput > 0)
				pszret = szInput;
			
			iInput = 0;
			return pszret;
		}

		szInput[iInput++] = ch;
	}

	return NULL;
}
#endif

#ifdef _WIN32
/*
==============
WriteStatusText

==============
*/
void WriteStatusText( char *szText )
{
	char szFullLine[81];
	COORD coord;
	DWORD dwWritten = 0;
	WORD wAttrib[80];
	
	int i;
	
	for ( i = 0; i < 80; i++ )
	{
		wAttrib[i] = FOREGROUND_RED | FOREGROUND_INTENSITY;
	}

	memset( szFullLine, 0, 81 );
	strcpy( szFullLine, szText );

	coord.X = 0;
	coord.Y = 0;

	WriteConsoleOutputAttribute( houtput, wAttrib, 80, coord, &dwWritten );
	WriteConsoleOutputCharacter( houtput, szFullLine, 80, coord, &dwWritten );	
}
#endif

/*
==============
CreateConsoleWindow

Create console window ( overridable? )
==============
*/
int CreateConsoleWindow( void )
{
#ifdef _WIN32
	if ( !AllocConsole () )
	{
		return 0;
	}

	hinput	= GetStdHandle (STD_INPUT_HANDLE);
	houtput = GetStdHandle (STD_OUTPUT_HANDLE);
	
	InitConProc();
#endif

	return 1;
}

/*
==============
DestroyConsoleWindow

==============
*/
void DestroyConsoleWindow( void )
{
#ifdef _WIN32
	FreeConsole ();

	// shut down QHOST hooks if necessary
	DeinitConProc ();
#endif
}

/*
==============
ProcessConsoleInput

==============
*/
void ProcessConsoleInput( void )
{
	char *s;

	if ( !engineapi.Cbuf_AddText )
		return;

	do
	{
		s = Sys_ConsoleInput ();
		if (s)
		{
			char szBuf[ 256 ];
			_snprintf(szBuf, sizeof(szBuf), "%s\n", s);
			engineapi.Cbuf_AddText ( szBuf );
		}
	} while (s);
}

/*
================
GameInit
================
*/
int GameInit(void)
{
	char *p;
#ifdef _WIN32
	MEMORYSTATUSEX Buffer;

	memset( &Buffer, 0, sizeof( Buffer ) );
	Buffer.dwLength = sizeof( MEMORYSTATUS );
	
	GlobalMemoryStatusEx ( &Buffer );

	// take the greater of all the available memory or half the total memory,
	// but at least 10 Mb and no more than 32 Mb, unless they explicitly
	// request otherwise
	giMemSize = Buffer.ullTotalPhys;

	if ( giMemSize < FIFTEEN_MEGS )	
		return 0;	

	if ( giMemSize < (int)( Buffer.ullTotalPhys >> 1 ) )	
		giMemSize = (int)( Buffer.ullTotalPhys >> 1 );	

	// At least 10 mb, even if we have to swap a lot.
	if (giMemSize <= MINIMUM_WIN_MEMORY)
	{
		giMemSize = MINIMUM_WIN_MEMORY;
	}
	else if (giMemSize > MAXIMUM_WIN_MEMORY)
	{
		giMemSize = MAXIMUM_WIN_MEMORY;
	}
#endif

	// Command line override
	if ( (CheckParm ("-heapsize", &p ) ) && p )
	{
		giMemSize = atoi( p ) * 1024;
	}

	// Command line to force running with minimal memory.
	if (CheckParm ("-minmemory", NULL))
	{
		giMemSize = MINIMUM_WIN_MEMORY;
	}
	
	// Try and allocated it
#ifdef _WIN32
	gpMemBase = (unsigned char *)::GlobalAlloc( GMEM_FIXED, giMemSize );
#else
	gpMemBase = (unsigned char *)malloc( giMemSize );

#endif

	if (!gpMemBase)
	{
		return 0;
	}

	if ( !Eng_Load( gpszCmdLine, &ef, giMemSize, gpMemBase, g_pszengine, DLL_NORMAL ) )
	{
		return 0;
	}

	Eng_SetState( DLL_ACTIVE );

	return 1;
}

/*
==============
GameShutdown

==============
*/
void GameShutdown( void )
{
	Eng_Unload();

	if ( gpMemBase )
	{
#ifdef _WIN32
		::GlobalFree( gpMemBase );
#else
		free( gpMemBase );
#endif
		gpMemBase = NULL;
	}
}

#define MAX_LINUX_CMDLINE 512 

static char cmdline[MAX_LINUX_CMDLINE];

void BuildCmdLine(int argc, char **argv)
{
	int len;
	int i;

	for (len = 0, i = 1; i < argc; i++)
	{
		len += strlen(argv[i]) + 1;
	}

	if (len > MAX_LINUX_CMDLINE)
	{
		printf("command line too long, %i max\n", MAX_LINUX_CMDLINE);
		exit(-1);
		return;
	}

	cmdline[0] = '\0';
	for (i = 1; i < argc; i++)
	{
		if (i > 1)
		{
			strcat(cmdline, " ");
		}
		strcat(cmdline, argv[i]);
	}
}

int main(int argc, char **argv)
{
	int		iret = -1;

#ifdef _DEBUG
	strcpy(g_szEXEName, "hlds_run.dbg");
#else
	strcpy(g_szEXEName, *argv);
#endif

	// Store off command line for argument searching
	BuildCmdLine(argc, argv);
	gpszCmdLine = strdup(cmdline);

	if (!InitInstance())
	{
		goto cleanup;
	}

	if (!CreateConsoleWindow())
	{
		goto cleanup;
	}

	if (!GameInit())
	{
		goto cleanup;
	}

	if (engineapi.SetStartupMode)
	{
		engineapi.SetStartupMode(1);
	}

	while (1)
	{
		char *p;
		static double oldtime = 0.0;

		double newtime;
		double dtime;

		// Try to allow other apps to get some CPU
		Sys_Sleep(1);

		if (!engineapi.Sys_FloatTime)
			break;

		while (1)
		{
			newtime = engineapi.Sys_FloatTime();
			if (newtime < oldtime)
			{
				oldtime = newtime - 0.05;
			}

			dtime = newtime - oldtime;

			if (dtime > 0.001)
				break;

			// Running really fast, yield some time to other apps
			Sys_Sleep(1);
		}

#ifdef _WIN32
		ProcessConsoleInput();

		if (engineapi.Host_Frame)
		{
			Eng_Frame(0, dtime);
		}

		UpdateStatus(0  /* don't force */);
#else
		Eng_Frame(0, dtime);
		p = Sys_ConsoleInput();
		if (p)
		{
			engineapi.Cbuf_AddText(p);
			engineapi.Cbuf_AddText("\n");
		}
#endif

		oldtime = newtime;
	}

	GameShutdown();

	DestroyConsoleWindow();

	iret = 1;

cleanup:

	if (gpszCmdLine)
	{
		free(gpszCmdLine);
	}

	if (hDLLThirdParty)
	{
		Sys_FreeLibrary(hDLLThirdParty);
		hDLLThirdParty = 0L;
	}

	return iret;
}
