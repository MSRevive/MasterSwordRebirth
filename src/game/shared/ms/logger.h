#ifndef MS_LOGGER
#define MS_LOGGER

#include <string>
#include <fstream>
#include <iostream>
#include "sharedutil.h"

class Logger {
public:
	enum e_logtype {
		LOG_ERROR,
		LOG_WARN,
		LOG_INFO
	};
	
	void open(const char *filename, int mode = 0)
	{
		switch(mode)
		{
		case 0:
			file.open(filename);
			if (file.is_open())
			{
				time_t Time;
				time(&Time);
				char *TimeString = ctime(&Time);
				
				file << (TimeString) << std::endl;
			}
			break;
		case 1:
			file.open(filename, std::ios_base::app); //open file for appending
			break;
		}
	}
	
	void close() 
	{
		if (file.is_open())
		{
			file.close();
		}
	}
	
	bool is_open()
	{
		return file.is_open();
	}
	
	friend Logger &operator<<(Logger &logger, const e_logtype ltype) 
	{
		switch (ltype) {
		case Logger::e_logtype::LOG_ERROR:
			logger.file << "[ERROR]: ";
			break;
		case Logger::e_logtype::LOG_WARN:
			logger.file << "[WARN]: ";
			break;
		case Logger::e_logtype::LOG_INFO:
			logger.file << "[INFO]: ";
			break;
		}
		
		return logger;
	}
	
	//std::endl now works
	friend Logger &operator<<(Logger &logger, std::ostream& (*os)(std::ostream&))
	{
		logger.file << os;
		return logger;
	}
	
	friend Logger &operator<<(Logger &logger, const char *text)
	{
		logger.file << text;
		//logger.file.flush();
		return logger;
	}
	
	friend Logger &operator<<(Logger &logger, unsigned char c)
	{
		logger.file << c;
		//logger.file.flush();
		return logger;
	}

	friend Logger &operator<<(Logger &logger, std::string str)
	{
		logger.file << str;
		return logger;
	}
	
	friend Logger &operator<<(Logger &logger, int n)
	{
		logger.file << n;
		//logger.file.flush();
		return logger;
	}
	
	friend Logger &operator<<(Logger &logger, unsigned long n)
	{
		logger.file << n;
		//logger.file.flush();
		return logger;
	}
	
	friend Logger &operator<<(Logger &logger, double n)
	{
		logger.file << n;
		//logger.file.flush();
		return logger;
	}
	
private:
	std::ofstream file;
};

extern Logger logfile;
#ifdef VALVE_DLL
extern Logger chatlog;
#endif
extern Logger NullFile;
extern bool g_log_initialized;

void Print(const char* szFmt, ...);
void Log(char* szFmt, ...);
void OpenLogFiles();

#define LogCurrentLine(Text) Log("%s:%i %s", __FILE__, __LINE__, msstring_ref(Text))
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

#endif