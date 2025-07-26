#ifndef MS_LOGGER_H
#define MS_LOGGER_H

#include <memory>
#include <string>
#include <stdarg.h>

// Forward declarations to avoid including spdlog in header
namespace spdlog {
    class logger;
}

class MSLogger {
public:
    // Log levels matching current system and spdlog
    enum Level {
        LEVEL_TRACE = 0,
        LEVEL_DEBUG = 1,
        LEVEL_INFO = 2,
        LEVEL_WARN = 3,
        LEVEL_ERROR = 4,
        LEVEL_CRITICAL = 5,
        LEVEL_OFF = 6
    };
    
    // Categories for different subsystems
    enum Category {
        GENERAL,
        SCRIPT,
        NETWORK,
        ENTITY,
        AI,
        CHAT,
        ANGELSCRIPT,
        PHYSICS,
        RENDER,
        SOUND,
        CATEGORY_COUNT
    };

    // Alert type compatibility for ALERT macro replacement
    enum AlertType {
        AT_NOTICE = 0,
        AT_CONSOLE = 1,
        AT_AICONSOLE = 2,
        AT_WARNING = 3,
        AT_ERROR = 4,
        AT_LOGGED = 5
    };
    
    // Initialize the logging system
    static void Initialize(const char* gameDir, bool isServer);
    
    // Shutdown and flush all loggers
    static void Shutdown();
    
    // Get logger for specific category
    static std::shared_ptr<spdlog::logger> GetLogger(Category cat);
    
    // Set log level for a category
    static void SetLevel(Category cat, Level level);
    
    // Set global log level
    static void SetGlobalLevel(Level level);
    
    // Convenience logging methods with printf-style formatting
    static void Error(Category cat, const char* fmt, ...);
    static void Warn(Category cat, const char* fmt, ...);
    static void Info(Category cat, const char* fmt, ...);
    static void Debug(Category cat, const char* fmt, ...);
    static void Trace(Category cat, const char* fmt, ...);
    
    // Direct string logging (no formatting)
    static void LogString(Category cat, Level level, const std::string& msg);
    
    // Special methods for compatibility with existing code
    static void Alert(AlertType type, const char* fmt, ...);
    static void Console(const char* fmt, ...);
    static void Chat(const char* fmt, ...);
    
    // File logging compatibility
    static void LogToFile(const char* filename, const char* fmt, ...);
    
    // Flush all loggers
    static void Flush();
    
    // Check if logging is initialized
    static bool IsInitialized() { return s_initialized; }
    
    // Get category name
    static const char* GetCategoryName(Category cat);
    
private:
    static bool s_initialized;
    static bool s_isServer;
    static std::shared_ptr<spdlog::logger> s_loggers[CATEGORY_COUNT];
    static std::shared_ptr<spdlog::logger> s_errorLogger;
    static std::shared_ptr<spdlog::logger> s_chatLogger;
    
    // Format string with va_list
    static std::string FormatString(const char* fmt, va_list args);
    
    // Map alert type to log level
    static Level AlertTypeToLevel(AlertType type);
    
    // Initialize individual logger
    static void InitializeLogger(Category cat, const std::string& logPath);
};

// Global convenience macros for easy migration
#define MS_ERROR(fmt, ...) MSLogger::Error(MSLogger::GENERAL, fmt, ##__VA_ARGS__)
#define MS_WARN(fmt, ...) MSLogger::Warn(MSLogger::GENERAL, fmt, ##__VA_ARGS__)
#define MS_INFO(fmt, ...) MSLogger::Info(MSLogger::GENERAL, fmt, ##__VA_ARGS__)
#define MS_DEBUG(fmt, ...) MSLogger::Debug(MSLogger::GENERAL, fmt, ##__VA_ARGS__)
#define MS_TRACE(fmt, ...) MSLogger::Trace(MSLogger::GENERAL, fmt, ##__VA_ARGS__)

// Category-specific macros
#define MS_SCRIPT_ERROR(fmt, ...) MSLogger::Error(MSLogger::SCRIPT, fmt, ##__VA_ARGS__)
#define MS_SCRIPT_INFO(fmt, ...) MSLogger::Info(MSLogger::SCRIPT, fmt, ##__VA_ARGS__)
#define MS_AI_DEBUG(fmt, ...) MSLogger::Debug(MSLogger::AI, fmt, ##__VA_ARGS__)
#define MS_ANGEL_INFO(fmt, ...) MSLogger::Info(MSLogger::ANGELSCRIPT, fmt, ##__VA_ARGS__)
#define MS_ANGEL_ERROR(fmt, ...) MSLogger::Error(MSLogger::ANGELSCRIPT, fmt, ##__VA_ARGS__)
#define MS_ANGEL_DEBUG(fmt, ...) MSLogger::Debug(MSLogger::ANGELSCRIPT, fmt, ##__VA_ARGS__)

// Compatibility macro for gradual ALERT migration
// Note: Engine headers should not be included here to avoid circular dependencies
// ALERT replacement is optional and can be enabled by defining MS_LOGGER_REPLACE_ALERT
#ifdef MS_LOGGER_REPLACE_ALERT
    #undef ALERT
    #define ALERT(type, ...) MSLogger::Alert((MSLogger::AlertType)type, __VA_ARGS__)
#endif

// Legacy compatibility functions
void MSLoggerPrint(const char* fmt, ...);
void MSLoggerLog(const char* fmt, ...);

#endif // MS_LOGGER_H