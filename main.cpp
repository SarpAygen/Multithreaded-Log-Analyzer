#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <utility>
#include <algorithm>

struct LogEntry {
    std::string timestamp;
    std::string severity;
    std::string message;
};

bool parseLogEntry(const std::string& line, LogEntry& entry) {
    std::istringstream stream(line);
    std::string date;
    std::string time;

    if (!(stream >> date >> time >> entry.severity)) {
        return false;
    }

    if (date.size() != 10 || time.size() != 8) {
        return false;
    }

    if (date[4] != '-' || date[7] != '-' ||
        time[2] != ':' || time[5] != ':') {
        return false;
    }

    if (date.find_first_not_of("0123456789", 0) != 4 ||
        date.find_first_not_of("0123456789", 5) != 7 ||
        date.find_first_not_of("0123456789", 8) != std::string::npos) {
        return false;
    }

    if (time.find_first_not_of("0123456789", 0) != 2 ||
        time.find_first_not_of("0123456789", 3) != 5 ||
        time.find_first_not_of("0123456789", 6) != std::string::npos) {
        return false;
    }

    entry.timestamp = date + " " + time;

    std::getline(stream, entry.message);
    if (entry.message.empty() || entry.message.front() != ' ') {
        return false;
    }

    entry.message.erase(0, 1);

    if (entry.message.find_first_not_of(" \t\r\n") == std::string::npos) {
        return false;
    }

    if (entry.severity != "INFO" &&
        entry.severity != "WARN" &&
        entry.severity != "ERROR") {
        return false;
    }

    if (line != entry.timestamp + " " + entry.severity + " " + entry.message) {
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {

    if (argc == 2 && std::string(argv[1]) == "--help") {
        std::cout << "Usage: log_analyzer.exe <log-file> [--severity LEVEL] [--keyword TEXT]\n";
        std::cout << "Read a log file and summarize valid entries and errors.\n";
        std::cout << "  --severity LEVEL  Display only INFO, WARN, or ERROR entries.\n";
        std::cout << "Summary counts include all valid entries, regardless of filters.\n";
        std::cout << "  --keyword TEXT    Display messages containing TEXT (case-sensitive).\n";
        std::cout << "                    Quote TEXT if it contains spaces.\n";
        return 0;
    }

    if (argc < 2 || argc % 2 != 0) {
        std::cerr << "Usage: log_analyzer.exe <log-file> [--severity LEVEL]\n";
        return 1;
    }

    std::string filePath = argv[1];
    std::ifstream logFile(filePath);

    if (!logFile.is_open()) {
        std::cerr << "Could not open " << filePath << '\n';
        return 1;
    }

    std::string line;

    int lineCount = 0;
    int malformedCount = 0;
    int infoCount = 0;
    int warnCount = 0;
    int errorCount = 0;
    std::string keywordFilter = "";
    std::string severityFilter = "";
    std::string startTime = "";
    std::string endTime = "";
    std::map<std::string, int> errorCounts;

    for (int i = 2; i < argc; i += 2) {
        if (std::string(argv[i]) == "--keyword") {
            keywordFilter = argv[i + 1];
            continue;
        }

        if (std::string(argv[i]) != "--severity") {
            std::cerr << "Unknown option: " << argv[i] << '\n';
            return 1;
        }

        severityFilter = argv[i + 1];

        if (severityFilter != "INFO" &&
            severityFilter != "WARN" &&
            severityFilter != "ERROR") {
            std::cerr << "Severity must be INFO, WARN, or ERROR.\n";
            return 1;
        }
    }

    if (!startTime.empty() && !endTime.empty() && startTime > endTime) {
        std::cerr << "Start time must not be after end time.\n";
        return 1;
    }

    while (std::getline(logFile, line)) {
        ++lineCount;
        LogEntry entry;

        if (parseLogEntry(line, entry)) {
            if (entry.severity == "INFO") {
            ++infoCount;
        } else if (entry.severity == "WARN") {
            ++warnCount;
        } else if (entry.severity == "ERROR") {
            ++errorCount;
            ++errorCounts[entry.message];
        }


        if ((severityFilter.empty() || entry.severity == severityFilter) && (keywordFilter.empty() || entry.message.find(keywordFilter) != std::string::npos) && (startTime.empty() || entry.timestamp >= startTime) && (endTime.empty() || entry.timestamp <= endTime)) {
            std::cout << entry.severity << ": " << entry.message << '\n';
        }

        } else {
            std::cerr << "Malformed entry on line " << lineCount << '\n';
            ++malformedCount;
        }
    }

    if (logFile.bad()) {
        std::cerr << "An error occurred while reading sample.log\n";
        return 1;
    }

    std::cout << "INFO entries: " << infoCount << '\n';
    std::cout << "WARN entries: " << warnCount << '\n';
    std::cout << "ERROR entries: " << errorCount << '\n';
    std::cout << "Malformed entries: " << malformedCount << '\n';

    std::vector<std::pair<std::string, int>> sortedErrors(
        errorCounts.begin(), errorCounts.end());

    std::sort(sortedErrors.begin(), sortedErrors.end(),
        [](const auto& a, const auto& b) {
            if (a.second != b.second) {
                return a.second > b.second;
            }
            return a.first < b.first;
        });

    std::cout << "Error message counts:\n";

    for (const auto& item : sortedErrors) {
        std::cout << item.second << " occurrence(s): " << item.first << '\n';
    }

    if (lineCount == 0) {
        std::cout << "The log file is empty.\n";
    }

    return 0;
}