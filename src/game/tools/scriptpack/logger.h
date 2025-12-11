#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <format>

class Logger {
public:
	// *** 1. Global Access Method (The Singleton Gateway) ***
	static Logger& GetInstance();

	// *** 2. Configuration Methods ***
    static void SetLogFile(const std::string& filename);
    // New method to control logging dynamically
    void EnableFileLogging(bool enable); 

	void RawLog(const std::string& message);

    // *** 3. Main Logging Method ***
    void Log(const std::string& message);

	template <typename... Args>
	void Log(std::format_string<Args...> fmt, Args&&... args)
	{
		std::string message = std::vformat(fmt.get(), std::make_format_args(args...));
		this->Log(message);
	}

	// *** 3. Prevent Copying/Moving ***
	// Required for the Singleton pattern
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
	Logger(Logger&&) = delete;
	Logger& operator=(Logger&&) = delete;

private:
	// *** 4. Private Constructor ***
	// Opens the file stream once when the Singleton is created.
	Logger();

	// Member variables
	std::ofstream logFileStream;
	bool isFileLoggingEnabled = true;
	static std::string logFilename;

	// Helper to get the current timestamp string
	std::string getCurrentTimestamp();
};