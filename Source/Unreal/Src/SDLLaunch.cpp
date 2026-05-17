#include "SDL2/SDL.h"
#ifdef PLATFORM_WIN32
#include <windows.h>
#endif
#ifdef PLATFORM_PSVITA
#include <vitasdk.h>
#include <vitaGL.h>
#include <unistd.h>
#endif
#ifdef PLATFORM_FUNKEY
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#endif

#include "Engine.h"

extern CORE_API FGlobalPlatform GTempPlatform;
extern DLL_IMPORT UBOOL GTickDue;
extern "C" {HINSTANCE hInstance;}
extern "C" {char GCC_HIDDEN THIS_PACKAGE[64]="Launch";}

// FExecHook.
class FExecHook : public FExec
{
	UBOOL Exec( const char* Cmd, FOutputDevice* Out )
	{
		return 0;
	}
};

FExecHook GLocalHook;
DLL_EXPORT FExec* GThisExecHook = &GLocalHook;

#ifdef PLATFORM_PSVITA

//
// PSVita-specific globals.
//

#define MAX_PATH 1024
#define SYSTEM_PATH "data/unreal/System/"

// 200MB libc heap, 512K main thread stack, 16MB for loading game DLLs
// the rest goes to vitaGL
extern "C" { SceUInt32 sceUserMainThreadStackSize = 512 * 1024; }
extern "C" { unsigned int _pthread_stack_default_user = 512 * 1024; }
extern "C" { unsigned int _newlib_heap_size_user = 200 * 1024 * 1024; }
#define VGL_MEM_THRESHOLD ( 16 * 1024 * 1024 )

static char GRootPath[MAX_PATH] = "app0:/";

//
// PSVita-specific functions.
//

static bool FindRootPath( char* Out, int OutLen )
{
	static const char *Drives[] = { "uma0", "imc0", "ux0" };

	// check if an unreal folder exists on one of the drives
	// default to the last one (ux0)
	for ( unsigned int i = 0; i < sizeof(Drives) / sizeof(*Drives); ++i )
	{
		snprintf( Out, OutLen, "%s:/" SYSTEM_PATH, Drives[i] );
		SceUID Dir = sceIoDopen( Out );
		if ( Dir >= 0 )
		{
			sceIoDclose( Dir );
			return true;
		}
	}

	// not found
	return false;
}

static INT PowerCallback( INT NotifyID, INT NotifyCnt, INT PowerInfo, void* Common )
{
	if ( PowerInfo & ( SCE_POWER_CB_APP_RESUME | SCE_POWER_CB_APP_RESUMING ) )
	{
		debugf( "PowerCallback: resuming..." );
		appHandleSuspendResume( false );
	}
	else if ( PowerInfo & ( SCE_POWER_CB_BUTTON_PS_PRESS | SCE_POWER_CB_APP_SUSPEND | SCE_POWER_CB_SYSTEM_SUSPEND ) )
	{
		debugf( "PowerCallback: suspending..." );
		appHandleSuspendResume( true );
	}

	return 0;
}

static INT CallbackThread( DWORD Argc, void* Argv )
{
	const INT CbID = sceKernelCreateCallback( "Power Callback", 0, PowerCallback, nullptr );
	scePowerRegisterCallback( CbID );
	while( true )
		sceKernelDelayThreadCB( 10000000 );
	return 0;
}

[[noreturn]] static void EarlyError( const char* Msg )
{
	fprintf( stderr, "FATAL ERROR: %s\n", Msg );
	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Fatal Error", Msg, nullptr );
	sceKernelExitProcess( 0 );
	abort();
}

static void PlatformPreInit()
{
	sceTouchSetSamplingState( SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_STOP );
	scePowerSetArmClockFrequency( 444 );
	scePowerSetBusClockFrequency( 222 );
	scePowerSetGpuClockFrequency( 222 );
	scePowerSetGpuXbarClockFrequency( 166 );
	sceSysmoduleLoadModule( SCE_SYSMODULE_NET );

	if ( !FindRootPath( GRootPath, sizeof(GRootPath) ) )
		EarlyError( "Could not find Unreal directory" );

	if ( chdir( GRootPath ) < 0 )
		EarlyError( "Could not chdir to Unreal directory" );

	SceUID Th = sceKernelCreateThread( "CallbackThread", CallbackThread, 0x10000100, 0x10000, 0, 0, nullptr );
	if( Th >= 0 )
		sceKernelStartThread( Th, 0, nullptr );

	vglInitWithCustomThreshold( 0, 960, 544, VGL_MEM_THRESHOLD, 0, 0, 0, SCE_GXM_MULTISAMPLE_2X );
	vglSetSemanticBindingMode( VGL_MODE_POSTPONED );
}

#elif defined(PLATFORM_FUNKEY)

//
// FunKey/RG Nano-specific globals and functions.
//

#define MAX_PATH 1024
#define SYSTEM_PATH "System/"

static char GRootPath[MAX_PATH] = "";

//
// Find the Unreal game directory on the FunKey filesystem.
//
static bool FindRootPath( char* Out, int OutLen )
{
	static const char* SearchPaths[] = {
		"/mnt/FunKey/Unreal/System/",
		"/mnt/Funkey/Unreal/System/",
		"/mnt/funkey/Unreal/System/",
		"./System/",
	};

	for( unsigned int i = 0; i < sizeof(SearchPaths) / sizeof(*SearchPaths); ++i )
	{
		DIR* Dir = opendir( SearchPaths[i] );
		if( Dir )
		{
			closedir( Dir );
			// Strip the trailing "System/" to get the root
			int len = strlen( SearchPaths[i] );
			if( len > 7 )
			{
				snprintf( Out, OutLen, "%.*s", len - 7, SearchPaths[i] );
			}
			else
			{
				snprintf( Out, OutLen, "./" );
			}
			return true;
		}
	}

	// Try current directory
	DIR* Dir = opendir( "." );
	if( Dir )
	{
		closedir( Dir );
		snprintf( Out, OutLen, "./" );
		return true;
	}

	return false;
}

[[noreturn]] static void EarlyError( const char* Msg )
{
	fprintf( stderr, "FATAL ERROR: %s\n", Msg );
	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Fatal Error", Msg, nullptr );
	_exit( 1 );
}

static void CrashHandler( int sig )
{
	const char* name = (sig == SIGSEGV) ? "SIGSEGV" : (sig == SIGBUS) ? "SIGBUS" : (sig == SIGABRT) ? "SIGABRT" : (sig == SIGILL) ? "SIGILL" : "UNKNOWN";
	fprintf( stderr, "CRASH: signal %s (%d) received\n", name, sig );
	fflush( stderr );
	fflush( stdout );
	signal( sig, SIG_DFL );
	raise( sig );
}

static void PlatformPreInit()
{
	signal( SIGSEGV, CrashHandler );
	signal( SIGBUS, CrashHandler );
	signal( SIGABRT, CrashHandler );
	signal( SIGILL, CrashHandler );

	// Set SDL to use directfb (no X11/Wayland on FunKey, fbdev via DirectFB)
	setenv( "SDL_VIDEODRIVER", "directfb", 0 );
	setenv( "SDL_AUDIODRIVER", "alsa", 0 );

	if( !FindRootPath( GRootPath, sizeof(GRootPath) ) )
		EarlyError( "Could not find Unreal directory.\nPlace game files at /mnt/FunKey/Unreal/ with a System/ subfolder." );

	// The engine expects CWD to be the System/ directory (where .u, .ini, .int files live).
	static char SystemPath[512];
	snprintf( SystemPath, sizeof(SystemPath), "%sSystem/", GRootPath );
	if( chdir( SystemPath ) < 0 )
		EarlyError( "Could not chdir to System directory" );

	// Redirect stdout/stderr to log file in the game root (not System/).
	static char LogPath[512];
	snprintf( LogPath, sizeof(LogPath), "%sfunkey.log", GRootPath );
	FILE* LogFile = freopen( LogPath, "w", stdout );
	if( LogFile )
	{
		freopen( LogPath, "a", stderr );
		// Line-buffer stdout so log lines are flushed immediately.
		// Without this, a crash loses all buffered output.
		setvbuf( stdout, NULL, _IOLBF, 0 );
	}

	fprintf( stdout, "FunKey: Using root path: %s\n", GRootPath );
	fprintf( stdout, "FunKey: Working directory: %s\n", SystemPath );
	fflush( stdout );
}

#else

static void PlatformPreInit()
{
	// Generic Linux/SDL — no special init needed.
}

#endif


//
// Handle an error.
//
void HandleError()
{
	GIsGuarded=0;
	GIsCriticalError=1;
	debugf( NAME_Exit, "Shutting down after catching exception" );
	GObj.ShutdownAfterError();
	debugf( NAME_Exit, "Exiting due to exception" );
	GErrorHist[ARRAY_COUNT(GErrorHist)-1]=0;
	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, LocalizeError("Critical"), GErrorHist, SDL_GetKeyboardFocus() );
}

//
// Initialize.
//
UEngine* InitEngine()
{
	guard(InitEngine);

	// Platform init.
	appInit();
	GDynMem.Init( 65536 );
	GSceneMem.Init( 32768 );

	// First-run menu.
	UBOOL FirstRun=0;
	GetConfigBool( "FirstRun", "FirstRun", FirstRun );

	// Create the global engine object.
	UClass* EngineClass;
	if( !GIsEditor )
	{
		// Create game engine.
		EngineClass = GObj.LoadClass( UGameEngine::StaticClass, NULL, "ini:Engine.Engine.GameEngine", NULL, LOAD_NoFail | LOAD_KeepImports, NULL );
	}
	else if( ParseParam( appCmdLine(),"MAKE" ) )
	{
		// Create editor engine.
		EngineClass = GObj.LoadClass( UEngine::StaticClass, NULL, "ini:Engine.Engine.EditorEngine", NULL, LOAD_NoFail | LOAD_DisallowFiles | LOAD_KeepImports, NULL );
	}
	else
	{
		// Editor.
		EngineClass = GObj.LoadClass( UEngine::StaticClass, NULL, "ini:Engine.Engine.EditorEngine", NULL, LOAD_NoFail | LOAD_KeepImports, NULL );
	}

	// Init engine.
	UEngine* Engine = ConstructClassObject<UEngine>( EngineClass );
	Engine->Init();

	return Engine;

	unguard;
}

//
// Unreal's main message loop.  All windows in Unreal receive messages
// somewhere below this function on the stack.
//
void MainLoop( UEngine* Engine )
{
	guard(MainLoop);

	GIsRunning = 1;
	DOUBLE OldTime = appSeconds();
	while( GIsRunning && !GIsRequestingExit )
	{
		// Update the world.
		DOUBLE NewTime = appSeconds();
		Engine->Tick( NewTime - OldTime );
		OldTime = NewTime;

		// Enforce optional maximum tick rate.
		INT MaxTickRate = Engine->GetMaxTickRate();
		if( MaxTickRate )
		{
			DOUBLE Delta = (1.0/MaxTickRate) - (appSeconds()-OldTime);
			if( Delta > 0.0 )
				appSleep( Delta );
		}
	}
	GIsRunning = 0;
	unguard;
}

//
// Exit the engine.
//
void ExitEngine( UEngine* Engine )
{
	guard(ExitEngine);

	// Save all config files (Unreal.ini etc.) before tearing down objects.
	// The normal path saves via GConfigCache destructor after main() returns,
	// but if anything crashes during shutdown the destructor never runs.
	GConfigCache.SaveAllConfigs();

	GObj.Exit();
	GMem.Exit();
	GDynMem.Exit();
	GSceneMem.Exit();
	GCache.Exit(1);
	appDumpAllocs( &GTempPlatform );

	unguard;
}

#ifdef PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInInstance, HINSTANCE hPrevInstance, char* InCmdLine, INT nCmdShow )
#else
int main( int argc, const char** argv )
#endif
{
#ifdef PLATFORM_WIN32
	hInstance = hInInstance;
#else
	hInstance = NULL;
	// Remember arguments since we don't have GetCommandLine().
	appSetCmdLine( argc, argv );
	PlatformPreInit();
#endif

	GIsStarted = 1;

	// Set package name.
	appStrcpy( THIS_PACKAGE, appPackage() );

	// Init mode.
	GIsServer = 1;
	GIsClient = !ParseParam(appCmdLine(),"SERVER") && !ParseParam(appCmdLine(),"MAKE");
	GIsEditor = ParseParam(appCmdLine(),"EDITOR") || ParseParam(appCmdLine(),"MAKE");

	// Init windowing.
	appChdir( appBaseDir() );

	// Init log.
	// TODO: GLog
	GExecHook = GThisExecHook;

	// Begin.
#ifndef _DEBUG
	try
	{
#endif
		// Start main loop.
		GIsGuarded=1;
		GSystem = &GTempPlatform;
		UEngine* Engine = InitEngine();
		if( !GIsRequestingExit )
			MainLoop( Engine );
		ExitEngine( Engine );
		GIsGuarded=0;
#ifndef _DEBUG
	}
	catch( ... )
	{
		// Crashed.
		try {HandleError();} catch( ... ) {}
	}
#endif

	// Shut down.
	GExecHook=NULL;
	appExit();
	GIsStarted = 0;
	return 0;
}
