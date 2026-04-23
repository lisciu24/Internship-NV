#ifndef LOG_ANALYZER_HPP
#define LOG_ANALYZER_HPP

#include <string>
#include <chrono>
#include <vector>
#include <deque>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <ranges>
#include <algorithm>
#include <format>

namespace LogAnalyzer
{

    struct LogEntry
    {
        std::chrono::system_clock::time_point timestamp;
        uint8_t log_level_id;
        uint8_t source_id;
        size_t msg_offset;
        size_t msg_lenght;
    };

    struct Query
    {
        std::optional<std::chrono::system_clock::time_point> start_time;
        std::optional<std::chrono::system_clock::time_point> end_time;
        std::optional<uint8_t> log_level_id;
        std::optional<uint8_t> source_id;
        std::optional<std::string> msg_substr;
    };

    class LogQueryEngine
    {
    public:
        Query parseQuery(const std::string &query_str);
        size_t executeQuery(const Query &query, std::ostream &os);
        int loadEntriesFromFile(const std::string &filename);

    private:
        std::string m_msg_buffer;
        std::vector<LogEntry> m_entries;

        std::unordered_map<uint8_t, std::vector<size_t>> m_log_level_entries_map;
        std::unordered_map<std::string_view, uint8_t> m_log_level_map;
        std::deque<std::string> m_log_level_names;

        std::unordered_map<uint8_t, std::vector<size_t>> m_source_entries_map;
        std::unordered_map<std::string_view, uint8_t> m_source_map;
        std::deque<std::string> m_source_names;

    private:
        void printFormatted(std::ostream &os, const LogEntry &entry);
        std::chrono::system_clock::time_point parseTimestamp(std::string_view ts);
        uint8_t getLogLevelId(std::string_view log_level_sv);
        uint8_t getSourceId(std::string_view source_sv);
    };
}

#endif // LOG_ANALYZER_HPP