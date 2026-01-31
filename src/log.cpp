#include "log.h"

#include <cstdlib>
#include <iostream>

DebugLog::DebugLog() {

    log_path = "./debug.log";

    // Open log file (append mode)
    log_file.open(log_path, std::ios::out | std::ios::app);

    if (log_file.is_open()) {
        log_file << "=== neo debug log started ===" << std::endl;
    }
}

void DebugLog::log(const std::string& msg) {
    if (log_file.is_open()) {
        log_file << msg << std::endl;
        log_file.flush();
    }
}
