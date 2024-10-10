#include "../headers.h"

/**
* @brief Constructs the Logger and creates necessary directories and log file.
* @param seederIp The IP address of the seeder.
* @param seederPort The port number of the seeder.
* @param name The name of the log file.
* @throws string If directory or file creation fails.
*/
Logger::Logger(string seederIp, int seederPort, string name)
: m_seederIp(seederIp)
, m_seederPort(to_string(seederPort))
, m_logDirPath("./logs/" + seederIp + ":" + to_string(seederPort))
, m_logFilePath(m_logDirPath + "/" + name + ".txt")
{
    // TODO : FIXME : Race condition is occuring, when this function is checking stats 
    
    struct stat info;

    //: Create base directory if it does not exist
    if (stat("./logs", &info) != 0 || !(info.st_mode & S_IFDIR)) {
        if (mkdir("./logs", 0755) != 0) {
            throw string("Making base directory for log!!");
        }
    }

    //: Create specific directory for this seeder if it does not exist
    if (stat(m_logDirPath.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
        if (mkdir(m_logDirPath.c_str(), 0755) != 0) {
            throw string("Making new directory for log!!");
        }
    }

    //: Create the log file
    int fd = open(m_logFilePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd <= 0){
        throw string("Opening log file!!");
    }
    close(fd);
}

/**
* @brief Logs a message with a timestamp and type to the log file.
* @param type The type of the log message (e.g., "ERROR", "INFO").
* @param content The content of the log message.
* @details The method uses a mutex to ensure thread-safe access to the log file. 
*          A timestamp is added to each log entry. 
*/
void Logger::log(string type, string content) {
    {
        if(content.empty() || content[content.size() - 1] == '\n') {
            content.pop_back(); // Remove trailing newline if present
        }

        time_t current_time = time(nullptr);
        struct tm* local_time = localtime(&current_time);
        char buffer[100];
        strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", local_time);
        string time = buffer;

        lock_guard<mutex> guard(m_logMutex);

        int fd = open(m_logFilePath.c_str(), O_WRONLY | O_APPEND, 0644);
        if (fd == -1) {
            return;
        }

        string temp = "\n[" + time + "][" + type + "] " + content;

        int bytesWritten = write(fd, temp.c_str(), temp.size());
        if (bytesWritten == -1) {
            close(fd);
            return;
        }

        close(fd);
    }
}
