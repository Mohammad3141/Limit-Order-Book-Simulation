#ifndef UTILS_H
#define UTILS_H

#include "DataTypes.h"
#include <string>
#include <vector>

namespace Utils
{
    // Parses a CSV line into an Event
    Event parse_line(const std::string &line, bool is_trade_file);

    // Human-readable nanosecond timestamp > IST string
    std::string format_timestamp_ist(uint64_t total_nanos);

    // Fast string splitter for CSV
    std::vector<std::string> split(const std::string &s, char delimiter);
}

#endif // UTILS_H
