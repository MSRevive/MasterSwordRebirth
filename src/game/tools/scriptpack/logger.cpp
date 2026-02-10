#include "logger.h"
#include <sstream>
#include <ctime>
#include <iomanip>
#include <stdexcept>

std::string Logger::logFilename = "./output.log";

void Logger::SetLogFile(const std::string& filename) {
	Logger::logFilename = filename;
}

void Logger::EnableFileLogging(bool enable) {
	Logger::isFileLoggingEnabled = enable;
}

std::string Logger::getCurrentTimestamp() {
	std::time_t now = std::time(nullptr);
	std::tm* ltm = std::localtime(&now);

	std::stringstream ss;
	ss << std::put_time(ltm, "%Y-%m-%d %H:%M:%S"); 
	return ss.str();
}

Logger::Logger() {
	logFileStream.open(logFilename, std::ios::trunc);

	if (!logFileStream.is_open()) {
		std::cerr << "[LOGGER INIT ERROR] Could not open log file: " << logFilename 
					<< ". Logging will be console-only." << std::endl;
	} else {
		logFileStream << "--- Logger Initialized at " << getCurrentTimestamp() << " ---\n";
		logFileStream.flush();
	}
}

Logger& Logger::GetInstance() {
	static Logger instance;
	return instance;
}

void Logger::RawLog(const std::string& message) {
	if (isFileLoggingEnabled && logFileStream.is_open()) {
		logFileStream << message << std::endl;
	}

	std::cout << message << std::endl;
}

void Logger::Log(const std::string& message) {
	std::string timestamp = getCurrentTimestamp();
	std::string fullMessage = "[" + timestamp + "] " + message;

	if (isFileLoggingEnabled && logFileStream.is_open()) {
		logFileStream << fullMessage << std::endl;
	}

	std::cout << fullMessage << std::endl;
}