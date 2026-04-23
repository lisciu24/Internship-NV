#include "LogAnalyzer.hpp"

namespace LogAnalyzer
{
    Query LogQueryEngine::parseQuery(const std::string &query_str)
    {
        Query q;

        std::string_view sv(query_str);
        size_t pos = 0;

        while (pos < sv.length())
        {
            size_t next = sv.find(';', pos);
            std::string_view pair = sv.substr(pos, next - pos);

            size_t eq_pos = pair.find('=');
            if (eq_pos != std::string_view::npos)
            {
                std::string_view key = pair.substr(0, eq_pos);
                std::string_view value = pair.substr(eq_pos + 1);

                if (key == "From")
                {
                    q.start_time = parseTimestamp(value);
                }
                else if (key == "To")
                {
                    q.end_time = parseTimestamp(value);
                }
                else if (key == "Source")
                {
                    auto tmp = m_source_map.find(value);
                    if (tmp != m_source_map.end())
                        q.source_id = tmp->second;
                }
                else if (key == "Log_Level")
                {
                    auto tmp = m_log_level_map.find(value);
                    if (tmp != m_log_level_map.end())
                        q.log_level_id = tmp->second;
                }
                else if (key == "Message")
                {
                    q.msg_substr = value;
                }
            }

            if (next == std::string_view::npos)
                break;

            pos = next + 1;
        }

        return q;
    }

    size_t LogQueryEngine::executeQuery(const Query &query, std::ostream &os)
    {
        auto toEntries = [this](auto &&idxs)
        {
            return idxs | std::views::transform([this](const size_t idx)
                                                { return m_entries[idx]; });
        };

        auto process = [this, &query, &os](auto &&entries)
        {
            auto it_start = query.start_time.has_value() ? std::ranges::lower_bound(entries, query.start_time.value(), {}, &LogEntry::timestamp) : entries.begin();
            auto it_end = query.end_time.has_value() ? std::ranges::upper_bound(entries, query.end_time.value(), {}, &LogEntry::timestamp) : entries.end();
            auto results = std::ranges::subrange(it_start, it_end) |
                           std::views::filter([this, &query](const LogEntry &entry)
                                              { if(!query.msg_substr.has_value()) return true;
                                                    std::string_view sv(m_msg_buffer.data() + entry.msg_offset, entry.msg_lenght); 
                                                    return sv.find(query.msg_substr.value()) != std::string::npos; });

            size_t count = 0;
            for (const LogEntry &entry : results)
            {
                printFormatted(os, entry);
                count++;
            }
            return count;
        };

        if (query.log_level_id && query.source_id)
        {
            std::vector<size_t> idxs;
            std::ranges::set_intersection(m_log_level_entries_map[query.log_level_id.value()], m_source_entries_map[query.source_id.value()], std::back_inserter(idxs));
            return process(toEntries(idxs));
        }
        else if (query.log_level_id)
        {
            return process(toEntries(m_log_level_entries_map[query.log_level_id.value()]));
        }
        else if (query.source_id)
        {
            return process(toEntries(m_source_entries_map[query.source_id.value()]));
        }

        return process(m_entries);
    }

    int LogQueryEngine::loadEntriesFromFile(const std::string &filename)
    {
        std::ifstream file(filename);
        std::string line;

        if (!file.is_open())
            return 0;

        m_entries.reserve(1000);
        while (std::getline(file, line))
        {
            if (line.empty())
                continue;

            std::string_view line_view(line);

            size_t timestamp_end = line_view.find(']');
            auto timestamp = parseTimestamp(line_view.substr(1, timestamp_end - 2));

            size_t log_level_start = line_view.find('[', timestamp_end + 1);
            size_t log_level_end = line_view.find(']', log_level_start + 1);
            uint8_t log_level_id = getLogLevelId(line_view.substr(log_level_start + 1, log_level_end - log_level_start - 1));

            size_t source_start = line_view.find('[', log_level_end + 1);
            size_t source_end = line_view.find(']', source_start + 1);
            uint8_t source_id = getSourceId(line_view.substr(source_start + 1, source_end - source_start - 1));

            std::string_view msg = line_view.substr(source_end + 2);

            size_t msg_offset = m_msg_buffer.size();
            size_t msg_length = msg.length();
            m_msg_buffer.append(msg);

            m_entries.emplace_back(timestamp, log_level_id, source_id, msg_offset, msg_length);
            m_log_level_entries_map[log_level_id].push_back(m_entries.size() - 1);
            m_source_entries_map[source_id].push_back(m_entries.size() - 1);
        }

        return 1;
    }

    void LogQueryEngine::printFormatted(std::ostream &os, const LogEntry &entry)
    {
        auto tp_seconds = std::chrono::floor<std::chrono::seconds>(entry.timestamp);
        os << "[" << std::format("{:%Y-%m-%dT%H:%M:%S}", tp_seconds) << "] ";
        os << "[" << m_log_level_names[entry.log_level_id] << "] ";
        os << "[" << m_source_names[entry.source_id] << "] ";
        os << m_msg_buffer.substr(entry.msg_offset, entry.msg_lenght);
        os << "\n";
    }

    std::chrono::system_clock::time_point LogQueryEngine::parseTimestamp(std::string_view ts)
    {
        int y, m, d, h, min, s;
        std::from_chars(ts.data(), ts.data() + 4, y);
        std::from_chars(ts.data() + 5, ts.data() + 7, m);
        std::from_chars(ts.data() + 8, ts.data() + 10, d);
        std::from_chars(ts.data() + 11, ts.data() + 13, h);
        std::from_chars(ts.data() + 14, ts.data() + 16, min);
        std::from_chars(ts.data() + 17, ts.data() + 19, s);

        auto date = std::chrono::year(y) / m / d;

        return std::chrono::sys_days{date} +
               std::chrono::hours{h} +
               std::chrono::minutes{min} +
               std::chrono::seconds{s};
    }

    uint8_t LogQueryEngine::getLogLevelId(std::string_view log_level_sv)
    {
        auto it = m_log_level_map.find(log_level_sv);
        if (it != m_log_level_map.end())
        {
            return it->second;
        }

        m_log_level_names.emplace_back(log_level_sv);
        uint8_t new_id = static_cast<uint8_t>(m_log_level_names.size() - 1);
        m_log_level_map.emplace(m_log_level_names.back(), new_id);

        return new_id;
    }

    uint8_t LogQueryEngine::getSourceId(std::string_view source_sv)
    {
        auto it = m_source_map.find(source_sv);
        if (it != m_source_map.end())
        {
            return it->second;
        }

        m_source_names.emplace_back(source_sv);
        uint8_t new_id = static_cast<uint8_t>(m_source_names.size() - 1);
        m_source_map.emplace(m_source_names.back(), new_id);

        return new_id;
    }

}