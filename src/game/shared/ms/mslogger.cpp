#include "mslogger.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #define MKDIR(path) mkdir(path, 0755)
#endif

// Check if spdlog is available
#ifdef HAS_SPDLOG
    #include "spdlog/spdlog.h"
    #include "spdlog/sinks/basic_file_sink.h"
    #include "spdlog/sinks/rotating_file_sink.h"
    #include "spdlog/sinks/stdout_color_sinks.h"
    #include "spdlog/sinks/daily_file_sink.h"
    #define SPDLOG_AVAILABLE 1
#else
    // Fallback implementation without spdlog
    #define SPDLOG_AVAILABLE 0
    namespace spdlog {
        class logger {
        public:
            void info(const std::string& msg) { printf("[INFO] %s\n", msg.c_str()); }
            void warn(const std::string& msg) { printf("[WARN] %s\n", msg.c_str()); }
            void error(const std::string& msg) { printf("[ERROR] %s\n", msg.c_str()); }
            void debug(const std::string& msg) { printf("[DEBUG] %s\n", msg.c_str()); }
            void trace(const std::string& msg) { printf("[TRACE] %s\n", msg.c_str()); }
            void critical(const std::string& msg) { printf("[CRITICAL] %s\n", msg.c_str()); }
            void flush() {}
        };
        typedef std::shared_ptr<logger> logger_ptr;
    }
#endif

// Static member initialization
bool MSLogger::s_initialized = false;
bool MSLogger::s_isServer = false;
std::shared_ptr<spdlog::logger> MSLogger::s_loggers[MSLogger::CATEGORY_COUNT];
std::shared_ptr<spdlog::logger> MSLogger::s_errorLogger;
std::shared_ptr<spdlog::logger> MSLogger::s_chatLogger;

// Category names
static const char* g_categoryNames[] = {
    "GENERAL",
    "SCRIPT",
    "NETWORK",
    "ENTITY",
    "AI",
    "CHAT",
    "ANGELSCRIPT",
    "PHYSICS",
    "RENDER",
    "SOUND"
};

void MSLogger::Initialize(const char* gameDir, bool isServer) {
    if (s_initialized) {
        return;
    }
    
    // Clean up any existing spdlog loggers to prevent "already exists" errors
#if SPDLOG_AVAILABLE
    try {
        // Drop any existing loggers with our names
        for (int i = 0; i < CATEGORY_COUNT; i++) {
            spdlog::drop(GetCategoryName(static_cast<Category>(i)));
        }
        spdlog::drop("ERRORS");
        spdlog::drop("CHAT");
    } catch (...) {
        // Ignore cleanup errors
    }
#endif
    
    s_isServer = isServer;
    
    // Create logs directory
    std::string logDir = std::string(gameDir) + "/logs";
    MKDIR(logDir.c_str());
    
#if SPDLOG_AVAILABLE
    try {
        // Create console sink with color
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::debug);
        
        // Initialize category loggers
        for (int i = 0; i < CATEGORY_COUNT; i++) {
            Category cat = static_cast<Category>(i);
            std::string logPath = logDir + "/" + (isServer ? "server_" : "client_") + 
                                  std::string(GetCategoryName(cat)) + ".log";
            
            // Create sinks
            std::vector<spdlog::sink_ptr> sinks;
            sinks.push_back(console_sink);
            
            // File sink with daily rotation
            auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
                logPath, 0, 0, false, 7); // Keep 7 days of logs
            file_sink->set_level(spdlog::level::trace);
            sinks.push_back(file_sink);
            
            // Create logger
            auto logger = std::make_shared<spdlog::logger>(
                GetCategoryName(cat), sinks.begin(), sinks.end());
            logger->set_level(spdlog::level::trace);
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
            
            spdlog::register_logger(logger);
            s_loggers[i] = logger;
        }
        
        // Special error logger (logs all errors to one file)
        auto error_file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            logDir + "/" + (isServer ? "server_errors.log" : "client_errors.log"), true);
        s_errorLogger = std::make_shared<spdlog::logger>("ERRORS", error_file_sink);
        s_errorLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] %v");
        spdlog::register_logger(s_errorLogger);
        
        // Chat logger with monthly rotation
        if (isServer) {
            auto chat_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
                logDir + "/chat", 0, 0, false, 30); // Keep 30 days
            s_chatLogger = std::make_shared<spdlog::logger>("CHAT", chat_sink);
            s_chatLogger->set_pattern("[%Y-%m-%d %H:%M:%S] %v");
            spdlog::register_logger(s_chatLogger);
        }
        
        // Set default log level based on build configuration
#ifdef DEBUG
        spdlog::set_level(spdlog::level::debug);
#else
        spdlog::set_level(spdlog::level::info);
#endif
        
    } catch (const std::exception& ex) {
        printf("MSLogger initialization failed: %s\n", ex.what());
        // Fall back to console logging
    }
#else
    // Fallback implementation
    printf("MSLogger: spdlog not available, using fallback implementation\n");
    
    // Create dummy loggers
    for (int i = 0; i < CATEGORY_COUNT; i++) {
        s_loggers[i] = std::make_shared<spdlog::logger>();
    }
    s_errorLogger = std::make_shared<spdlog::logger>();
    if (isServer) {
        s_chatLogger = std::make_shared<spdlog::logger>();
    }
#endif
    
    s_initialized = true;
    Info(GENERAL, "MSLogger initialized successfully (%s mode)", isServer ? "server" : "client");
}

void MSLogger::Shutdown() {
    if (!s_initialized) {
        return;
    }
    
    Flush();
    
#if SPDLOG_AVAILABLE
    spdlog::shutdown();
#endif
    
    s_initialized = false;
}

std::shared_ptr<spdlog::logger> MSLogger::GetLogger(Category cat) {
    if (!s_initialized || cat < 0 || cat >= CATEGORY_COUNT) {
        return nullptr;
    }
    return s_loggers[cat];
}

void MSLogger::SetLevel(Category cat, Level level) {
#if SPDLOG_AVAILABLE
    auto logger = GetLogger(cat);
    if (logger) {
        logger->set_level(static_cast<spdlog::level::level_enum>(level));
    }
#endif
}

void MSLogger::SetGlobalLevel(Level level) {
#if SPDLOG_AVAILABLE
    spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
#endif
}

std::string MSLogger::FormatString(const char* fmt, va_list args) {
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    return std::string(buffer);
}

void MSLogger::Error(Category cat, const char* fmt, ...) {
    if (!s_initialized) return;
    
    va_list args;
    va_start(args, fmt);
    std::string msg = FormatString(fmt, args);
    va_end(args);
    
    auto logger = GetLogger(cat);
    if (logger) {
#if SPDLOG_AVAILABLE
        logger->error(msg);
#else
        logger->error(msg);
#endif
    }
    
    // Also log to error file
    if (s_errorLogger) {
#if SPDLOG_AVAILABLE
        s_errorLogger->error("[{}] {}", GetCategoryName(cat), msg);
#else
        s_errorLogger->error("[" + std::string(GetCategoryName(cat)) + "] " + msg);
#endif
    }
}

void MSLogger::Warn(Category cat, const char* fmt, ...) {
    if (!s_initialized) return;
    
    va_list args;
    va_start(args, fmt);
    std::string msg = FormatString(fmt, args);
    va_end(args);
    
    auto logger = GetLogger(cat);
    if (logger) {
#if SPDLOG_AVAILABLE
        logger->warn(msg);
#else
        logger->warn(msg);
#endif
    }
}

void MSLogger::Info(Category cat, const char* fmt, ...) {
    if (!s_initialized) return;
    
    va_list args;
    va_start(args, fmt);
    std::string msg = FormatString(fmt, args);
    va_end(args);
    
    auto logger = GetLogger(cat);
    if (logger) {
#if SPDLOG_AVAILABLE
        logger->info(msg);
#else
        logger->info(msg);
#endif
    }
}

void MSLogger::Debug(Category cat, const char* fmt, ...) {
    if (!s_initialized) return;
    
    va_list args;
    va_start(args, fmt);
    std::string msg = FormatString(fmt, args);
    va_end(args);
    
    auto logger = GetLogger(cat);
    if (logger) {
#if SPDLOG_AVAILABLE
        logger->debug(msg);
#else
        logger->debug(msg);
#endif
    }
}

void MSLogger::Trace(Category cat, const char* fmt, ...) {
    if (!s_initialized) return;
    
    va_list args;
    va_start(args, fmt);
    std::string msg = FormatString(fmt, args);
    va_end(args);
    
    auto logger = GetLogger(cat);
    if (logger) {
#if SPDLOG_AVAILABLE
        logger->trace(msg);
#else
        logger->trace(msg);
#endif
    }
}

void MSLogger::LogString(Category cat, Level level, const std::string& msg) {
    if (!s_initialized) return;
    
    auto logger = GetLogger(cat);
    if (!logger) return;
    
#if SPDLOG_AVAILABLE
    switch (level) {
        case LEVEL_TRACE: logger->trace(msg); break;
        case LEVEL_DEBUG: logger->debug(msg); break;
        case LEVEL_INFO: logger->info(msg); break;
        case LEVEL_WARN: logger->warn(msg); break;
        case LEVEL_ERROR: logger->error(msg); break;
        case LEVEL_CRITICAL: logger->critical(msg); break;
        default: break;
    }
#else
    // Fallback
    switch (level) {
        case LEVEL_TRACE: logger->trace(msg); break;
        case LEVEL_DEBUG: logger->debug(msg); break;
        case LEVEL_INFO: logger->info(msg); break;
        case LEVEL_WARN: logger->warn(msg); break;
        case LEVEL_ERROR: logger->error(msg); break;
        case LEVEL_CRITICAL: logger->critical(msg); break;
        default: break;
    }
#endif
}

MSLogger::Level MSLogger::AlertTypeToLevel(AlertType type) {
    switch (type) {
        case AT_NOTICE: return LEVEL_INFO;
        case AT_CONSOLE: return LEVEL_INFO;
        case AT_AICONSOLE: return LEVEL_DEBUG;
        case AT_WARNING: return LEVEL_WARN;
        case AT_ERROR: return LEVEL_ERROR;
        case AT_LOGGED: return LEVEL_INFO;
        default: return LEVEL_INFO;
    }
}

void MSLogger::Alert(AlertType type, const char* fmt, ...) {
    if (!s_initialized) {
        // Fallback to printf if not initialized
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        return;
    }
    
    va_list args;
    va_start(args, fmt);
    std::string msg = FormatString(fmt, args);
    va_end(args);
    
    Level level = AlertTypeToLevel(type);
    Category cat = GENERAL;
    
    // Special handling for AI console
    if (type == AT_AICONSOLE) {
        cat = AI;
    }
    
    LogString(cat, level, msg);
    
    // For AT_LOGGED, also log to network category
    if (type == AT_LOGGED) {
        LogString(NETWORK, level, msg);
    }
}

void MSLogger::Console(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::string msg = FormatString(fmt, args);
    va_end(args);
    
    if (s_initialized) {
        Info(GENERAL, "%s", msg.c_str());
    } else {
        printf("%s\n", msg.c_str());
    }
}

void MSLogger::Chat(const char* fmt, ...) {
    if (!s_initialized || !s_chatLogger) return;
    
    va_list args;
    va_start(args, fmt);
    std::string msg = FormatString(fmt, args);
    va_end(args);
    
#if SPDLOG_AVAILABLE
    s_chatLogger->info(msg);
#else
    s_chatLogger->info(msg);
#endif
}

void MSLogger::LogToFile(const char* filename, const char* fmt, ...) {
    // For compatibility with old file-based logging
    va_list args;
    va_start(args, fmt);
    std::string msg = FormatString(fmt, args);
    va_end(args);
    
    // Log to general category with filename prefix
    Info(GENERAL, "[%s] %s", filename, msg.c_str());
}

void MSLogger::Flush() {
    if (!s_initialized) return;
    
#if SPDLOG_AVAILABLE
    spdlog::apply_all([](std::shared_ptr<spdlog::logger> l) { l->flush(); });
#else
    // Fallback - flush stdout
    fflush(stdout);
#endif
}

const char* MSLogger::GetCategoryName(Category cat) {
    if (cat < 0 || cat >= CATEGORY_COUNT) {
        return "UNKNOWN";
    }
    return g_categoryNames[cat];
}

// Legacy compatibility functions
void MSLoggerPrint(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    MSLogger::Console("%s", buffer);
}

void MSLoggerLog(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    MSLogger::Debug(MSLogger::GENERAL, "%s", buffer);
}