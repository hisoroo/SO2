#include "Utils.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <mutex>
#include <iostream>
#include <filesystem> 

namespace {
    std::mutex fileMutex;
}

std::string getLogFilename() {
    const std::string dir = "chat_logs";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;
    localtime_r(&now_time_t, &local_tm);
    std::ostringstream oss;
    oss << dir << "/chat_log_" << std::put_time(&local_tm, "%d-%m-%Y") << ".txt";
    return oss.str();
}

void saveMessageToFile(const std::string &message) {
    std::lock_guard<std::mutex> lock(fileMutex);
    std::string filename = getLogFilename();
    std::ofstream outFile(filename, std::ios::app);
    if (outFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm;
        localtime_r(&now_time_t, &local_tm);
        std::ostringstream timeStream;
        timeStream << std::put_time(&local_tm, "%H:%M:%S");
        outFile << "[" << timeStream.str() << "] " << message << "\n";
    } else {
        std::cerr << "Error: Unable to open " << filename << "\n";
    }
}

