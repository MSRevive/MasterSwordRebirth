//When defined, DEV_BUILD indicates that this is
//a MSDev version of MS.  Certain features are allowed in this mode
//New: This is now defined in the project instead of a file
//#define DEV_BUILD

//When defined, the client and server dlls will maintain txt logfiles
#define KEEP_LOG

//Allows exceptions to be logged to both console and the logfile
#define LOG_EXCEPTIONS
//!!!EXTENSIVE_LOGGING has MOVED!! - Look at the top of Global.cpp!
//#define EXTENSIVE_LOGGING		//Causes EXTENSIVE logging of every dbg operation

#ifdef EXTENSIVE_LOGGING
#define LOG_INPUTS //Automatic, with extensive logging
#endif

//Causes DbgInputs() to be called for each DLL input function in the client and server dlls
#define TRACK_INPUTS
//#define LOG_INPUTS				//Extensive logging of inputs to file

//In the release build, whether certain errors treated as fatal (release) or not (debug)
#define RELEASE_LOCKDOWN

#ifdef RELEASE_LOCKDOWN
#define SCRIPT_LOCKDOWN //Automatic, with lockdown mode
#endif

// SILENCE SOME WARNINGS

#ifdef _WIN32

// Remove warnings from warning level 4.
#pragma warning(disable : 4514) // warning C4514: 'acosl' : unreferenced inline function has been removed
#pragma warning(disable : 4100) // warning C4100: 'hwnd' : unreferenced formal parameter
#pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#pragma warning(disable : 4512) // warning C4512: 'InFileRIFF' : assignment operator could not be generated
#pragma warning(disable : 4611) // warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable
#pragma warning(disable : 4710) // warning C4710: function 'x' not inlined
#pragma warning(disable : 4702) // warning C4702: unreachable code
#pragma warning(disable : 4505) // unreferenced local function has been removed
#pragma warning(disable : 4239) // nonstandard extension used : 'argument' ( conversion from class Vector to class Vector& )
#pragma warning(disable : 4097) // typedef-name 'BaseClass' used as synonym for class-name 'CFlexCycler::CBaseFlex'
#pragma warning(disable : 4324) // Padding was added at the end of a structure
#pragma warning(disable : 4244) // type conversion warning.
#pragma warning(disable : 4305) // truncation from 'const double ' to 'float '
#pragma warning(disable : 4786) // Disable warnings about long symbol names
#pragma warning(disable : 4250) // 'X' : inherits 'Y::Z' via dominance
#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable : 4481) // warning C4481: nonstandard extension used: override specifier 'override'
#pragma warning(disable : 4748) // warning C4748: /GS can not protect parameters and local variables from local buffer overrun because optimizations are disabled in function

#if _MSC_VER >= 1300
#pragma warning(disable : 4511) // Disable warnings about private copy constructors
#pragma warning(disable : 4121) // warning C4121: 'symbol' : alignment of a member was sensitive to packing
#pragma warning(disable : 4530) // warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc (disabled due to std headers having exception syntax)
#endif

#if _MSC_VER >= 1400
#pragma warning(disable : 4996) // functions declared deprecated
#endif

#endif // _WIN32

#if !defined(_WIN64)
#pragma warning(disable : 4267) // conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable : 4311) // pointer truncation from 'char *' to 'int'
#pragma warning(disable : 4312) // conversion from 'unsigned int' to 'memhandle_t' of greater size
#endif
