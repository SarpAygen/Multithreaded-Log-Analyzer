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

bool isValidTimestamp(const std::string& timestamp) {

    if (timestamp.size() != 19) {
        return false;
    }

    if (timestamp[4] != '-' || timestamp[7] != '-' ||
        timestamp[10] != ' ' ||
        timestamp[13] != ':' || timestamp[16] != ':') {
        return false;
    }

    for (std::size_t i = 0; i < timestamp.size(); ++i) {
        if (i == 4 || i == 7 || i == 10 || i == 13 || i == 16) {
            continue;
        }

        if (timestamp[i] < '0' || timestamp[i] > '9') {
            return false;
        }
    }
    return true;
}

bool parseLogEntry(const std::string& line, LogEntry& entry) {

    std::istringstream stream(line);
    std::string date;
    std::string time;

    if (!(stream >> date >> time >> entry.severity)) {
        return false;
    }

    entry.timestamp = date + " " + time;

    if (!isValidTimestamp(entry.timestamp)) {
        return false;
    }

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

std::string escapeCsv(const std::string& text) {
    std::string result = "\"";

    for (char character : text) {
        if (character == '"') {
            result += '"';
        }

        result += character;
    }

    result += '"';
    return result;
}

int main(int argc, char* argv[]) {

    if (argc == 2 && std::string(argv[1]) == "--help") {
        std::cout << "Usage: log_analyzer.exe <log-file> [--severity LEVEL] [--keyword TEXT] [--start TIME] [--end TIME] [--csv PATH]\n";
        std::cout << "Read a log file and summarize valid entries and errors.\n";
        std::cout << "  --severity LEVEL  Display only INFO, WARN, or ERROR entries.\n";
        std::cout << "Summary counts include all valid entries, regardless of filters.\n";
        std::cout << "  --keyword TEXT    Display messages containing TEXT (case-sensitive).\n";
        std::cout << "                    Quote TEXT if it contains spaces.\n";
        std::cout << "  --start TIME      Include entries at or after TIME.\n";
        std::cout << "  --end TIME        Include entries at or before TIME.\n";
        std::cout << "                    Format: \"YYYY-MM-DD HH:MM:SS\" (quote the value).\n";
        std::cout << "  --csv PATH        Write all summary counts and ranked errors to CSV.\n";
        std::cout << "                    Replaces the output file if it exists.\n";
        return 0;
    }

    if (argc < 2 || argc % 2 != 0) {
        std::cerr << "Usage: log_analyzer.exe <log-file> [--severity LEVEL] [--keyword TEXT] [--start TIME] [--end TIME] [--csv PATH]\n";
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
    std::string csvPath = "";
    std::map<std::string, int> errorCounts;

    for (int i = 2; i < argc; i += 2) {

        if (std::string(argv[i]) == "--csv") {
            csvPath = argv[i + 1];

            if (csvPath.empty()) {
                std::cerr << "CSV output path must not be empty.\n";
                return 1;
            }

            continue;
        }

        if (std::string(argv[i]) == "--start") {
            startTime = argv[i + 1];
            continue;
        }

        if (std::string(argv[i]) == "--end") {
            endTime = argv[i + 1];
            continue;
        }

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

    if ((!startTime.empty() && !isValidTimestamp(startTime)) ||
        (!endTime.empty() && !isValidTimestamp(endTime))) {
        std::cerr << "Time boundaries must use YYYY-MM-DD HH:MM:SS.\n";
        return 1;
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

    if (!csvPath.empty()) {
        std::ofstream csvFile(csvPath);

        if (!csvFile.is_open()) {
            std::cerr << "Could not open CSV output: " << csvPath << '\n';
            return 1;
        }

        csvFile << "type,label,count\n";
        csvFile << "severity,INFO," << infoCount << '\n';
        csvFile << "severity,WARN," << warnCount << '\n';
        csvFile << "severity,ERROR," << errorCount << '\n';
        csvFile << "input,malformed," << malformedCount << '\n';

        for (const auto& item : sortedErrors) {
            csvFile << "error_message,"
                    << escapeCsv(item.first) << ','
                    << item.second << '\n';
        }

        csvFile.close();

        if (!csvFile) {
            std::cerr << "Failed to write CSV output: " << csvPath << '\n';
            return 1;
        }
    }
    return 0;
}