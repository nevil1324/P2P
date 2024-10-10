#include "../headers.h"

/**
* @brief Processes and validates command-line arguments.
* @param argc The argument count.
* @param argv The argument vector.
* @return A vector of strings containing the processed arguments.
* @throws string If the number of arguments is incorrect or if any argument is invalid.
*/
vector<string> Utils::processArgs(int argc, char *argv[]) {
    if (argc != 4) {
        throw string("Invalid arguments!!");
    }

    vector<string> temp;

    char *seederIpAndPort = argv[1];
    char *trackerInfoFileName = argv[2];
    int trackerNumber = atoi(argv[3]);

    vector<string> seederIpPortVec = tokenize(seederIpAndPort, ':');
    if (seederIpPortVec.size() != 2) {
        throw string("Invalid format of ip:port of seeder!!");
    }
    temp.push_back(seederIpPortVec[0]);
    temp.push_back(seederIpPortVec[1]);

    if (trackerNumber <= 0) {
        throw string("Tracker number is invalid!!");
    }

    int fd = open(trackerInfoFileName, O_RDONLY);
    if (fd < 0) {
        string s = trackerInfoFileName;
        throw string("Opening " + s + " file!!");
    }

    char buffer[524288];
    int bytesRead = read(fd, buffer, sizeof(buffer));
    if (bytesRead <= 0) {
        string s = trackerInfoFileName;
        throw string("Reading " + s + " file!!");
    }

    vector<string> ipAndPorts = tokenize(buffer, '\n');
    if (ipAndPorts.size() < trackerNumber) {
        throw string("IP and port of tracker number " + to_string(trackerNumber) + " is not defined in file!!");
    }

    string ipAndPort = ipAndPorts[trackerNumber - 1];
    vector<string> trackerIpPortVec = tokenize(ipAndPort, ':');
    if (trackerIpPortVec.size() != 2) {
        throw string("Invalid format of ip:port of tracker number " + to_string(trackerNumber) + "!!");
    }

    temp.push_back(trackerIpPortVec[0]);
    temp.push_back(trackerIpPortVec[1]);
    return temp;
}

/**
* @brief Tokenizes a string based on a specified separator.
* @param buffer The string to tokenize.
* @param separator The character to use as the delimiter.
* @return A vector of strings obtained by splitting the input string.
*/
vector<string> Utils::tokenize(string buffer, char separator) {
    vector<string> ans;
    string temp;
    for (auto it : buffer) {
        if (it == separator) {
            if (temp.size()) ans.push_back(temp);
            temp.clear();
        } else {
            temp.push_back(it);
        }
    }
    if (temp.size()) ans.push_back(temp);
    return ans;
}

/**
* @brief Computes the SHA-256 hashes of a file.
* @param filePath The path to the file.
* @return A vector of SHA-256 hashes, where the first entry is the hash of the entire file
*         and the subsequent entries are hashes of individual pieces.
* @throws string If there is an error opening or reading the file.
*/
vector<string> Utils::findSHA(string filePath) {
    int fileFd = open(filePath.c_str(), O_RDONLY);
    if (fileFd < 0) {
        throw string("Opening file at findSHA()!!\nError: " + string(strerror(errno)));
    }
    SHA256_CTX sha256_1;
    SHA256_Init(&sha256_1);

    vector<SHA256_CTX> temp;
    char buffer[PIECE_SIZE];
    ssize_t bytesRead;

    while ((bytesRead = read(fileFd, buffer, sizeof(buffer))) > 0) {
        SHA256_CTX sha256_2;
        SHA256_Init(&sha256_2);
        SHA256_Update(&sha256_1, buffer, bytesRead);
        SHA256_Update(&sha256_2, buffer, bytesRead);

        temp.push_back(sha256_2);
    }
    close(fileFd);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256_1);

    char hex_hash[2 * SHA256_DIGEST_LENGTH + 1];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hex_hash + 2 * i, "%02x", hash[i]);
    }

    vector<string> fileSHAs;
    fileSHAs.push_back(hex_hash);
    
    for (auto &it : temp) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &it);

        char hex_hash[2 * SHA256_DIGEST_LENGTH + 1];
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            sprintf(hex_hash + 2 * i, "%02x", hash[i]);
        }
        fileSHAs.push_back(hex_hash);
    }
    return fileSHAs;
}

/**
* @brief Computes the SHA-256 hash of a piece of data.
* @param pieceData The data for which to compute the hash.
* @return The SHA-256 hash as a hexadecimal string.
*/
string Utils::findPieceSHA(string pieceData) {
    SHA256_CTX sha256_2;
    SHA256_Init(&sha256_2);
    SHA256_Update(&sha256_2, pieceData.c_str(), pieceData.size());

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256_2);

    char hex_hash[2 * SHA256_DIGEST_LENGTH + 1];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hex_hash + 2 * i, "%02x", hash[i]);
    }

    return hex_hash;
}

/**
* @brief Retrieves the size of a file.
* @param filePath The path to the file.
* @return The size of the file in bytes.
* @throws string If there is an error opening or seeking the file.
*/
int Utils::giveFileSize(string filePath) {
    int fileFd = open(filePath.c_str(), O_RDONLY);
    if (fileFd < 0) {
        throw string("Opening file at giveFileSize()\nError: " + string(strerror(errno)));
    }

    off_t fileSize = lseek(fileFd, 0, SEEK_END);
    if (fileSize < 0) {
        close(fileFd);
        throw string("Seeking at giveFileSize()\nError: " + string(strerror(errno)));
    }

    close(fileFd);
    return fileSize;
}
