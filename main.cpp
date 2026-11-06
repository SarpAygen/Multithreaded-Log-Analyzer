#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::ifstream logFile("sample.log");

    if (!logFile.is_open()) {
        std::cerr << "Could not open sample.log\n";
        return 1;
    }

    std::string line;

    int lineCount = 0;

    while (std::getline(logFile, line)) {
        std::cout << line << '\n';
        ++lineCount;
    }
    if (lineCount == 0) {
        std::cout << "The log file is empty.\n";
    }
    return 0;
}