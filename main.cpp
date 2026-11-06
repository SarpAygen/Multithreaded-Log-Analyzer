#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

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

int main() {
    std::ifstream logFile("sample.log");

    if (!logFile.is_open()) {
        std::cerr << "Could not open sample.log\n";
        return 1;
    }

    std::string line;

    int lineCount = 0;
    int malformedCount = 0;

    while (std::getline(logFile, line)) {
        ++lineCount;
        LogEntry entry;

        if (parseLogEntry(line, entry)) {
            std::cout << entry.severity << ": " << entry.message << '\n';
        } else {
            std::cerr << "Malformed entry on line " << lineCount << '\n';
            ++malformedCount;
        }
    }

    std::cout << "Malformed entries: " << malformedCount << '\n';

    if (lineCount == 0) {
        std::cout << "The log file is empty.\n";
    }

    return 0;
}