#include "../headers.h"

Logger::Logger(string seederIp, int seederPort, string name)
: m_seederIp(seederIp)
, m_seederPort(to_string(seederPort))
, m_logDirPath("./logs/" + seederIp + ":" + to_string(seederPort))
, m_logFilePath(m_logDirPath + "/" + name + ".txt")
{
    //: Creating directory named logs_seederIp_seederPort
    struct stat info;

    if (stat("./logs", &info) != 0 || !(info.st_mode & S_IFDIR)) {
        if (mkdir("./logs", 0755) != 0) {
            throw string("Making base directory for log!!");
        }
    }

    if (stat(m_logDirPath.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
        if (mkdir(m_logDirPath.c_str(), 0755) != 0) {
            throw string("Making new directory for log!!");
        }
    }

    //: Creating leecher log file
    int fd = open(m_logFilePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd <= 0){
        throw string("Opening log file!!");
    }

    close(fd);
}

void Logger::log(string type, string content){

    {
        if(content[content.size() - 1] == '\n') content.pop_back();

        time_t current_time = time(nullptr);
        struct tm* local_time = localtime(&current_time);
        char buffer[100];
        strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", local_time);
        string time = buffer;

        lock_guard <mutex> guard(m_logMutex);
        
        int fd = open(m_logFilePath.c_str(), O_WRONLY | O_APPEND, 0644);
        if (fd == -1) {
            return;
        }

        string temp = "\n["+time+"]["+type+"] "+content;
        
        int bytesWritten = write(fd, temp.c_str(), temp.size());
        if (bytesWritten == -1) {
            close(fd);
            return;
        }
        
        close(fd);
    }
    
}