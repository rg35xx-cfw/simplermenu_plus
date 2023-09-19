#pragma once

#include <iostream>
#include <string>

enum LogLevel {
    NONE = 0,
    ERROR = 1,
    WARN = 2,
    INFO = 3,
    DEBUG = 4
};

class Logger {
private:
    LogLevel currentLevel;

public:
    Logger(LogLevel level = NONE) : currentLevel(level) {}

    void setLogLevel(LogLevel level) {
        currentLevel = level;
    }

    template<typename... Strings>
    void log(LogLevel level, Strings... messages) {
        if (level <= currentLevel) {
            print(messages...);
        }
    }

private:
    void print() {} // base case

    template<typename First, typename... Rest>
    void print(const First& first, const Rest&... rest) {
        std::cout << first;
        print(rest...);  // recursive call
    }
};

