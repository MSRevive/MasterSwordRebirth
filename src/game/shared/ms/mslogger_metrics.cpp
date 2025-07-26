#include "mslogger_metrics.h"
#include "mslogger_config.h"
#include <sstream>
#include <iomanip>

std::atomic<size_t> MSLoggerMemoryTracker::s_currentBytes{0};
std::atomic<size_t> MSLoggerMemoryTracker::s_peakBytes{0};

MSLoggerMetrics& MSLoggerMetrics::Instance() {
    static MSLoggerMetrics instance;
    return instance;
}

MSLoggerMetrics::MSLoggerMetrics() {
    m_perfStats.startTime = std::chrono::steady_clock::now();
    Reset();
}

void MSLoggerMetrics::RecordMessage(MSLogger::Category cat, MSLogger::Level level, size_t messageSize) {
    if (cat < 0 || cat >= MSLogger::CATEGORY_COUNT) return;
    
    auto now = std::chrono::steady_clock::now();
    
    // Update category stats
    m_categoryStats[cat].messageCount.fetch_add(1);
    m_categoryStats[cat].bytesLogged.fetch_add(messageSize);
    
    if (level == MSLogger::LEVEL_ERROR) {
        m_categoryStats[cat].errorsLogged.fetch_add(1);
    } else if (level == MSLogger::LEVEL_WARN) {
        m_categoryStats[cat].warningsLogged.fetch_add(1);
    }
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_categoryStats[cat].lastLogTime = now;
    }
    
    // Update global stats
    m_perfStats.totalMessages.fetch_add(1);
    m_perfStats.totalBytes.fetch_add(messageSize);
}

void MSLoggerMetrics::RecordDroppedMessage() {
    m_perfStats.droppedMessages.fetch_add(1);
}

void MSLoggerMetrics::RecordFlush(std::chrono::steady_clock::duration flushTime) {
    m_perfStats.flushCount.fetch_add(1);
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_perfStats.totalLogTime += flushTime;
}

MSLoggerMetrics::CategoryStats MSLoggerMetrics::GetCategoryStats(MSLogger::Category cat) const {
    if (cat < 0 || cat >= MSLogger::CATEGORY_COUNT) {
        return CategoryStats{};
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_categoryStats[cat];
}

MSLoggerMetrics::PerformanceStats MSLoggerMetrics::GetPerformanceStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_perfStats;
}

double MSLoggerMetrics::GetMessagesPerSecond() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_perfStats.startTime);
    
    if (elapsed.count() > 0) {
        return static_cast<double>(m_perfStats.totalMessages.load()) / elapsed.count();
    }
    return 0.0;
}

size_t MSLoggerMetrics::GetAverageMessageSize() const {
    uint64_t totalMessages = m_perfStats.totalMessages.load();
    if (totalMessages > 0) {
        return m_perfStats.totalBytes.load() / totalMessages;
    }
    return 0;
}

double MSLoggerMetrics::GetCategoryUsagePercent(MSLogger::Category cat) const {
    if (cat < 0 || cat >= MSLogger::CATEGORY_COUNT) return 0.0;
    
    uint64_t totalMessages = m_perfStats.totalMessages.load();
    if (totalMessages > 0) {
        uint64_t catMessages = m_categoryStats[cat].messageCount.load();
        return (static_cast<double>(catMessages) / totalMessages) * 100.0;
    }
    return 0.0;
}

void MSLoggerMetrics::Reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Reset category stats
    for (int i = 0; i < MSLogger::CATEGORY_COUNT; i++) {
        m_categoryStats[i].messageCount.store(0);
        m_categoryStats[i].bytesLogged.store(0);
        m_categoryStats[i].errorsLogged.store(0);
        m_categoryStats[i].warningsLogged.store(0);
    }
    
    // Reset performance stats
    m_perfStats.totalMessages.store(0);
    m_perfStats.totalBytes.store(0);
    m_perfStats.droppedMessages.store(0);
    m_perfStats.flushCount.store(0);
    m_perfStats.totalLogTime = std::chrono::steady_clock::duration{0};
    m_perfStats.startTime = std::chrono::steady_clock::now();
}

std::string MSLoggerMetrics::GenerateReport() const {
    std::stringstream report;
    
    report << "=== MSLogger Metrics Report ===\n";
    report << std::fixed << std::setprecision(2);
    
    // Overall statistics
    auto perfStats = GetPerformanceStats();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - perfStats.startTime);
    
    report << "\nOverall Statistics:\n";
    report << "  Total Messages: " << perfStats.totalMessages << "\n";
    report << "  Total Bytes: " << perfStats.totalBytes << " (" 
           << (perfStats.totalBytes / 1024.0 / 1024.0) << " MB)\n";
    report << "  Messages/Second: " << GetMessagesPerSecond() << "\n";
    report << "  Average Message Size: " << GetAverageMessageSize() << " bytes\n";
    report << "  Dropped Messages: " << perfStats.droppedMessages << "\n";
    report << "  Flush Operations: " << perfStats.flushCount << "\n";
    report << "  Runtime: " << elapsed.count() << " seconds\n";
    
    // Category breakdown
    report << "\nCategory Breakdown:\n";
    report << std::left << std::setw(15) << "Category" 
           << std::right << std::setw(10) << "Messages"
           << std::setw(10) << "Percent"
           << std::setw(10) << "Errors"
           << std::setw(10) << "Warnings"
           << std::setw(12) << "Bytes" << "\n";
    report << std::string(67, '-') << "\n";
    
    for (int i = 0; i < MSLogger::CATEGORY_COUNT; i++) {
        auto cat = static_cast<MSLogger::Category>(i);
        auto stats = m_categoryStats[i];
        
        if (stats.messageCount > 0) {
            report << std::left << std::setw(15) << MSLogger::GetCategoryName(cat)
                   << std::right << std::setw(10) << stats.messageCount
                   << std::setw(10) << GetCategoryUsagePercent(cat) << "%"
                   << std::setw(10) << stats.errorsLogged
                   << std::setw(10) << stats.warningsLogged
                   << std::setw(12) << stats.bytesLogged << "\n";
        }
    }
    
    // Memory usage
    report << "\nMemory Usage:\n";
    report << "  Current: " << (MSLoggerMemoryTracker::GetCurrentUsage() / 1024.0) << " KB\n";
    report << "  Peak: " << (MSLoggerMemoryTracker::GetPeakUsage() / 1024.0) << " KB\n";
    
    return report.str();
}

void MSLoggerMetrics::LogSummary() {
    MS_INFO("=== MSLogger Metrics Summary ===");
    MS_INFO("Total Messages: %llu", m_perfStats.totalMessages.load());
    MS_INFO("Messages/Second: %.2f", GetMessagesPerSecond());
    MS_INFO("Average Message Size: %zu bytes", GetAverageMessageSize());
    MS_INFO("Dropped Messages: %llu", m_perfStats.droppedMessages.load());
    
    // Log top categories
    struct CategoryUsage {
        MSLogger::Category cat;
        uint64_t count;
        double percent;
    };
    
    std::vector<CategoryUsage> usage;
    for (int i = 0; i < MSLogger::CATEGORY_COUNT; i++) {
        auto cat = static_cast<MSLogger::Category>(i);
        uint64_t count = m_categoryStats[i].messageCount.load();
        if (count > 0) {
            usage.push_back({cat, count, GetCategoryUsagePercent(cat)});
        }
    }
    
    // Sort by usage
    std::sort(usage.begin(), usage.end(), [](const CategoryUsage& a, const CategoryUsage& b) {
        return a.count > b.count;
    });
    
    MS_INFO("Top Categories:");
    for (size_t i = 0; i < std::min(usage.size(), size_t(5)); i++) {
        MS_INFO("  %s: %llu messages (%.1f%%)", 
                MSLogger::GetCategoryName(usage[i].cat),
                usage[i].count,
                usage[i].percent);
    }
}

// Memory tracker implementation
void MSLoggerMemoryTracker::RecordAllocation(size_t bytes) {
    size_t current = s_currentBytes.fetch_add(bytes) + bytes;
    
    // Update peak if necessary
    size_t peak = s_peakBytes.load();
    while (current > peak && !s_peakBytes.compare_exchange_weak(peak, current)) {
        // Loop until successful
    }
}

void MSLoggerMemoryTracker::RecordDeallocation(size_t bytes) {
    s_currentBytes.fetch_sub(bytes);
}

size_t MSLoggerMemoryTracker::GetCurrentUsage() {
    return s_currentBytes.load();
}

size_t MSLoggerMemoryTracker::GetPeakUsage() {
    return s_peakBytes.load();
}

void MSLoggerMemoryTracker::Reset() {
    s_currentBytes.store(0);
    s_peakBytes.store(0);
}