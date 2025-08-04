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

        friend Logger &operator<<(Logger &logger, msstring text)
	{
		logger.file << text.c_str();
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
void Log(const char* szFmt, ...);
void OpenLogFiles();


#endif
