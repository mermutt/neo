#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>

class DebugLog {
    std::ofstream log_file;
    std::string log_path;

public:
    DebugLog();
    void log(const std::string& msg);
};
