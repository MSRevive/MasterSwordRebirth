#ifndef MS_LOGGER_METRICS_H
#define MS_LOGGER_METRICS_H

#include "mslogger.h"
#include <chrono>
#include <atomic>
#include <unordered_map>
#include <mutex>

/**
 * MSLogger Metrics Extension
 * 
 * Provides performance metrics and statistics for the logging system.
 * Useful for monitoring logging overhead and optimizing performance.
 */

class MSLoggerMetrics {
public:
    struct CategoryStats {
        std::atomic<uint64_t> messageCount{0};
        std::atomic<uint64_t> bytesLogged{0};
        std::atomic<uint64_t> errorsLogged{0};
        std::atomic<uint64_t> warningsLogged{0};
        std::chrono::steady_clock::time_point lastLogTime;
    };
    
    struct PerformanceStats {
        std::atomic<uint64_t> totalMessages{0};
        std::atomic<uint64_t> totalBytes{0};
        std::atomic<uint64_t> droppedMessages{0};
        std::atomic<uint64_t> flushCount{0};
        std::chrono::steady_clock::time_point startTime;
        std::chrono::steady_clock::duration totalLogTime{0};
    };
    
    // Singleton instance
    static MSLoggerMetrics& Instance();
    
    // Record a log message
    void RecordMessage(MSLogger::Category cat, MSLogger::Level level, size_t messageSize);
    
    // Record a dropped message (queue full)
    void RecordDroppedMessage();
    
    // Record a flush operation
    void RecordFlush(std::chrono::steady_clock::duration flushTime);
    
    // Get statistics
    CategoryStats GetCategoryStats(MSLogger::Category cat) const;
    PerformanceStats GetPerformanceStats() const;
    
    // Get messages per second rate
    double GetMessagesPerSecond() const;
    
    // Get average message size
    size_t GetAverageMessageSize() const;
    
    // Get category usage percentage
    double GetCategoryUsagePercent(MSLogger::Category cat) const;
    
    // Reset all metrics
    void Reset();
    
    // Generate metrics report
    std::string GenerateReport() const;
    
    // Log metrics summary (uses MSLogger)
    void LogSummary();
    
private:
    MSLoggerMetrics();
    ~MSLoggerMetrics() = default;
    
    // Disable copy/move
    MSLoggerMetrics(const MSLoggerMetrics&) = delete;
    MSLoggerMetrics& operator=(const MSLoggerMetrics&) = delete;
    
    mutable std::mutex m_mutex;
    CategoryStats m_categoryStats[MSLogger::CATEGORY_COUNT];
    PerformanceStats m_perfStats;
};

/**
 * RAII Timer for measuring function execution time
 */
class MSLoggerTimer {
public:
    MSLoggerTimer(const char* functionName, MSLogger::Category cat = MSLogger::GENERAL)
        : m_functionName(functionName)
        , m_category(cat)
        , m_start(std::chrono::steady_clock::now()) {
    }
    
    ~MSLoggerTimer() {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start);
        
        if (duration.count() > 1000) { // Only log if > 1ms
            MSLogger::Debug(m_category, "%s took %lld µs", m_functionName, duration.count());
        }
    }
    
private:
    const char* m_functionName;
    MSLogger::Category m_category;
    std::chrono::steady_clock::time_point m_start;
};

/**
 * Scoped logger for tracking execution flow
 */
class MSLoggerScope {
public:
    MSLoggerScope(const char* scopeName, MSLogger::Category cat = MSLogger::GENERAL)
        : m_scopeName(scopeName)
        , m_category(cat) {
        MSLogger::Trace(m_category, ">>> Entering %s", m_scopeName);
    }
    
    ~MSLoggerScope() {
        MSLogger::Trace(m_category, "<<< Leaving %s", m_scopeName);
    }
    
private:
    const char* m_scopeName;
    MSLogger::Category m_category;
};

//==========================================================================
// Convenience Macros
//==========================================================================

#ifdef MS_LOGGER_ENABLE_METRICS

// Time a function or code block
#define MS_LOG_TIMER(name) MSLoggerTimer _timer##__LINE__(name)
#define MS_LOG_TIMER_CAT(name, cat) MSLoggerTimer _timer##__LINE__(name, cat)

// Track execution scope
#define MS_LOG_SCOPE(name) MSLoggerScope _scope##__LINE__(name)
#define MS_LOG_SCOPE_CAT(name, cat) MSLoggerScope _scope##__LINE__(name, cat)

// Log function entry/exit
#define MS_LOG_FUNC() MS_LOG_SCOPE(__FUNCTION__)
#define MS_LOG_FUNC_CAT(cat) MS_LOG_SCOPE_CAT(__FUNCTION__, cat)

#else

// No-op macros when metrics are disabled
#define MS_LOG_TIMER(name) ((void)0)
#define MS_LOG_TIMER_CAT(name, cat) ((void)0)
#define MS_LOG_SCOPE(name) ((void)0)
#define MS_LOG_SCOPE_CAT(name, cat) ((void)0)
#define MS_LOG_FUNC() ((void)0)
#define MS_LOG_FUNC_CAT(cat) ((void)0)

#endif // MS_LOGGER_ENABLE_METRICS

/**
 * Memory usage tracker
 */
class MSLoggerMemoryTracker {
public:
    static void RecordAllocation(size_t bytes);
    static void RecordDeallocation(size_t bytes);
    static size_t GetCurrentUsage();
    static size_t GetPeakUsage();
    static void Reset();
    
private:
    static std::atomic<size_t> s_currentBytes;
    static std::atomic<size_t> s_peakBytes;
};

#endif // MS_LOGGER_METRICS_H