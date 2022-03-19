#ifndef MS_LOGFILE
#define MS_LOGFILE

#include "buildcontrol.h"
#include "sharedutil.h"
#include <fstream>
#include <string>

using namespace std;

class CMSStream : public ofstream
{
public:
	ostream &operator<<(const char *_s)
	{
		if (!is_open())
		{
			buffered += _s;
			return *this;
		}
#ifdef _WIN32
		ostream &OStream = ::operator<<(*this, _s);
#else
		ostream &OStream = ostream::operator<<(_s);
#endif
		flush();
		return OStream;
	}
	ostream &operator<<(unsigned char _c)
	{
		if (!is_open())
			return *this;
		ostream &OStream = ostream::operator<<(_c);
		flush();
		return OStream;
	}
	ostream &operator<<(int _num)
	{
		if (!is_open())
			return *this;
		ostream &OStream = ostream::operator<<(_num);
		flush();
		return OStream;
	}
	ostream &operator<<(unsigned long _num)
	{
		if (!is_open())
			return *this;
		ostream &OStream = ostream::operator<<(_num);
		flush();
		return OStream;
	}
	ostream &operator<<(double _num)
	{
		if (!is_open())
			return *this;
		ostream &OStream = ostream::operator<<(_num);
		flush();
		return OStream;
	}
	/*ostream& operator << ( double _num )
	{
		ostream& OStream = ostream::operator << ( _num );
		flush( );
		return OStream;
	}*/
	void open(msstring_ref FileName);
	void open(msstring_ref FileName, int mode);
	void Close() { ofstream::close(); }
	void DebugOpen();

	static msstring buffered;
};
extern CMSStream logfile;
#ifdef VALVE_DLL
extern CMSStream chatlog;
#endif
extern CMSStream NullFile;
extern bool g_log_initialized;

enum
{ //  GET RID OF THIS!
	CLERR_BAD_ITEM_ID_IN_MSGFUNC_HANDS = 100,
	CLERR_BAD_ITEM_ID_IN_MSGFUNC_CONTAINER, //101
	CLERR_EXECPTION_HUD_WeaponsPostThink,	//102
	CLERR_EXECPTION_InitHud,				//103
	CLERR_EXECPTION_CHud_Think,				//104
	CLERR_EXECPTION_MsgFunc_Pack,			//105
	CLERR_EXECPTION_RestoreAll,				//106
	MSERR_EXCEPTION_ITEMTHINK,				//107
	MSERR_EXCEPTION_MONSTERTHINK,			//108
	MSERR_EXCEPTION_PLAYERPRETHINK,			//109
	MSERR_EXCEPTION_PLAYERPOSTTHINK,		//110
	MSERR_EXCEPTION_SERVERTHINK,			//111
};

void Print(char *szFmt, ...);
void Log(char *szFmt, ...);
const char *ShortProjectFileName(const char *FileName);

#define LogCurrentLine(Text) Log("%s:%i %s", ShortProjectFileName(__FILE__), __LINE__, msstring_ref(Text))
void LogExtensive(msstring_ref Text);
#define DBG_ENTR_FUNCTION_TEXT (msstring("Enter - ") + __FUNCTION__)
#define DBG_EXIT_FUNCTION_TEXT (msstring("Exit - ") + __FUNCTION__)

#ifdef LOG_EXCEPTIONS
#ifndef EXTENSIVE_LOGGING
#define SetDebugProgress(a, b) a = b
#else
#define SetDebugProgress(a, b) \
	a = b;                     \
	LogCurrentLine(a);
#endif

#define startdbg          \
	msstring FunctionPrg; \
	try                   \
	{                     \
		LogExtensive(DBG_ENTR_FUNCTION_TEXT)
#define enddbgline(a)                         \
	LogExtensive(DBG_EXIT_FUNCTION_TEXT + a); \
	}                                         \
	catch (...) { MSErrorConsoleText(msstring("Error: ") + __FUNCTION__ + a, FunctionPrg); }

/*#ifndef EXTENSIVE_LOGGING
		#define startdbg msstring FunctionPrg; try {
		#define enddbgline( a ) } catch( ... ) { MSErrorConsoleText( msstring(__FUNCTION__) + a, FunctionPrg ); }
	#else
		#define startdbg msstring FunctionPrg; try { LogCurrentLine( DBG_ENTR_FUNCTION_TEXT )
		#define enddbgline( a ) LogCurrentLine( DBG_EXIT_FUNCTION_TEXT + a ); } catch( ... ) { MSErrorConsoleText( msstring("Error: ") + __FUNCTION__ + a, FunctionPrg ); }
	#endif*/

#define dbg(a) SetDebugProgress(FunctionPrg, a)
#define enddbg enddbgline("")
#define enddbgprt(a) enddbgline(msstring(" - ") + a);
#else
#define startdbg
#define SetDebugProgress
#define dbg SetDebugProgress
#define enddbg
#define enddbgprt
#endif

void MSErrorConsoleText(const msstring_ref pszLabel, const msstring_ref Progress);

extern msstring ItemThinkProgress;
#endif //MS_LOGFILE