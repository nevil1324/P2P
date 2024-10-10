#include "../headers.h"

mutex Files::m_fileNameToFilePathMutex;
mutex Files::m_filePathToAvailablePiecesMutex;
map<pair<string, string>, string> Files::m_fileNameToFilePath;
map<string, vector<int>> Files::m_filePathToAvailablePieces;

/**
* @brief Adds a file path to the map with synchronization.
* @param fileName The name of the file.
* @param groupName The name of the group.
* @param filePath The path to the file.
*/
void Files::addFilepath(string fileName, string groupName, string filePath) {
    lock_guard<mutex> guard(Files::m_fileNameToFilePathMutex);
    m_fileNameToFilePath[{fileName, groupName}] = filePath;
}

/**
* @brief Adds a piece number to the list of available pieces for a given file path.
* @param filePath The path to the file.
* @param pieceNumber The piece number to add.
*/
void Files::addPieceToFilepath(string filePath, int pieceNumber) {
    lock_guard<mutex> guard(Files::m_filePathToAvailablePiecesMutex);
    m_filePathToAvailablePieces[filePath].push_back(pieceNumber);
}

/**
* @brief Retrieves the file path for a given file name and group name.
* @param fileName The name of the file.
* @param groupName The name of the group.
* @return The file path, or an empty string if not found.
*/
string Files::giveFilePath(string fileName, string groupName) {
    lock_guard<mutex> guard(Files::m_fileNameToFilePathMutex);
    auto it = m_fileNameToFilePath.find({fileName, groupName});
    return (it != m_fileNameToFilePath.end()) ? it->second : "";
}

/**
* @brief Retrieves a string of available pieces for a given file path.
* @param filePath The path to the file.
* @return A string of available piece numbers separated by spaces, or an empty string if none.
*/
string Files::giveAvailablePieces(string filePath) {
    lock_guard<mutex> guard(Files::m_filePathToAvailablePiecesMutex);
    auto it = m_filePathToAvailablePieces.find(filePath);
    if (it == m_filePathToAvailablePieces.end()) {
        return "";
    }

    string temp;
    for (const auto& pieceNumber : it->second) {
        temp.append(" " + to_string(pieceNumber));
    }
    return temp;
}

/**
* @brief Checks if a specific piece is available for a given file path.
* @param filePath The path to the file.
* @param pieceNumber The piece number to check.
* @return True if the piece is available, false otherwise.
*/
bool Files::isPieceAvailable(string filePath, int pieceNumber) {
    lock_guard<mutex> guard(Files::m_filePathToAvailablePiecesMutex);
    auto it = m_filePathToAvailablePieces.find(filePath);
    if (it == m_filePathToAvailablePieces.end()) {
        return false;
    }
    
    const auto& pieces = it->second;
    return find(pieces.begin(), pieces.end(), pieceNumber) != pieces.end();
}
