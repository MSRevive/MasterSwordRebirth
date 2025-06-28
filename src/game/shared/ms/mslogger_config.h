#ifndef MS_LOGGER_CONFIG_H
#define MS_LOGGER_CONFIG_H

/**
 * MSLogger Configuration Header
 * 
 * This file contains compile-time configuration options for MSLogger.
 * Modify these settings to customize logging behavior for your environment.
 */

//==========================================================================
// Feature Toggles
//==========================================================================

// Enable ALERT macro replacement globally
// Uncomment to replace all ALERT() calls with MSLogger
// #define MS_LOGGER_REPLACE_ALERT

// Enable performance metrics logging
#define MS_LOGGER_ENABLE_METRICS

// Enable log file compression (requires zlib)
// #define MS_LOGGER_ENABLE_COMPRESSION

//==========================================================================
// Default Log Levels
//==========================================================================

// Default log level for each category in debug builds
#ifdef DEBUG
    #define MS_LOGGER_DEFAULT_LEVEL MSLogger::LEVEL_DEBUG
    #define MS_LOGGER_DEFAULT_AI_LEVEL MSLogger::LEVEL_TRACE
    #define MS_LOGGER_DEFAULT_SCRIPT_LEVEL MSLogger::LEVEL_DEBUG
#else
    #define MS_LOGGER_DEFAULT_LEVEL MSLogger::LEVEL_INFO
    #define MS_LOGGER_DEFAULT_AI_LEVEL MSLogger::LEVEL_INFO
    #define MS_LOGGER_DEFAULT_SCRIPT_LEVEL MSLogger::LEVEL_INFO
#endif

//==========================================================================
// File Configuration
//==========================================================================

// Maximum size of a single log file before rotation (in MB)
#define MS_LOGGER_MAX_FILE_SIZE_MB 50

// Number of old log files to keep
#define MS_LOGGER_MAX_BACKUP_FILES 7

// Chat log retention days
#define MS_LOGGER_CHAT_RETENTION_DAYS 30

// Error log retention days
#define MS_LOGGER_ERROR_RETENTION_DAYS 14

//==========================================================================
// Performance Configuration
//==========================================================================

// Size of the async logging queue
#define MS_LOGGER_ASYNC_QUEUE_SIZE 8192

// Flush interval in seconds
#define MS_LOGGER_FLUSH_INTERVAL_SECONDS 3

// Maximum message length (characters)
#define MS_LOGGER_MAX_MESSAGE_LENGTH 4096

//==========================================================================
// Format Configuration
//==========================================================================

// Default log format pattern
#define MS_LOGGER_DEFAULT_PATTERN "[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v"

// Console format pattern
#define MS_LOGGER_CONSOLE_PATTERN "[%H:%M:%S] [%^%l%$] %v"

// Chat log format pattern
#define MS_LOGGER_CHAT_PATTERN "[%Y-%m-%d %H:%M:%S] %v"

//==========================================================================
// Memory Configuration (32-bit constraints)
//==========================================================================

// Maximum memory usage for logging system (in MB)
#define MS_LOGGER_MAX_MEMORY_MB 32

// Enable memory monitoring
#define MS_LOGGER_MONITOR_MEMORY

//==========================================================================
// Network Logging Configuration
//==========================================================================

// Enable remote logging support
// #define MS_LOGGER_ENABLE_REMOTE

// Remote logging server address
#define MS_LOGGER_REMOTE_HOST "127.0.0.1"
#define MS_LOGGER_REMOTE_PORT 9999

//==========================================================================
// Debug Features
//==========================================================================

// Enable source location logging (file:line)
#ifdef DEBUG
    #define MS_LOGGER_ENABLE_SOURCE_LOCATION
#endif

// Enable function name logging
// #define MS_LOGGER_ENABLE_FUNCTION_NAMES

// Enable thread ID logging
#define MS_LOGGER_ENABLE_THREAD_ID

//==========================================================================
// Integration Options
//==========================================================================

// Enable Steam integration for crash dumps
// #define MS_LOGGER_ENABLE_STEAM_DUMPS

// Enable automatic bug reporting
// #define MS_LOGGER_ENABLE_BUG_REPORTS

//==========================================================================
// Conditional Compilation Helpers
//==========================================================================

// Helper macros for conditional features
#ifdef MS_LOGGER_ENABLE_SOURCE_LOCATION
    #define MS_LOGGER_FILE __FILE__
    #define MS_LOGGER_LINE __LINE__
    #define MS_LOGGER_LOC_ARGS , const char* file, int line
    #define MS_LOGGER_LOC_PARAMS , MS_LOGGER_FILE, MS_LOGGER_LINE
#else
    #define MS_LOGGER_FILE ""
    #define MS_LOGGER_LINE 0
    #define MS_LOGGER_LOC_ARGS
    #define MS_LOGGER_LOC_PARAMS
#endif

//==========================================================================
// Platform-Specific Configuration
//==========================================================================

#ifdef _WIN32
    // Windows-specific settings
    #define MS_LOGGER_USE_WINDOWS_CONSOLE_COLORS
    #define MS_LOGGER_PATH_SEPARATOR "\\"
#else
    // Linux/Unix settings
    #define MS_LOGGER_USE_ANSI_COLORS
    #define MS_LOGGER_PATH_SEPARATOR "/"
#endif

//==========================================================================
// Deprecated Feature Warnings
//==========================================================================

// Warn about deprecated logging methods
#define MS_LOGGER_WARN_DEPRECATED

// Auto-convert deprecated calls
// #define MS_LOGGER_AUTO_CONVERT_DEPRECATED

#endif // MS_LOGGER_CONFIG_H