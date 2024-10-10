#include "../headers.h"

/**
 * @brief Initializes the Seeder by setting up the socket.
 * @return void
 */
void Seeder::init(){
    m_seederSocket.createSocket();
    m_seederSocket.setOptions();
    m_seederSocket.bindSocket();
    m_seederSocket.listenSocket();
    cout << "Seeder started listening!!\n" << flush;
    m_logger.log("Success", "Seeder started listening!!");
}

/**
 * @brief Starts the Seeder by creating and detaching a thread to accept connections.
 * 
 * This method spawns a new thread to handle incoming connections and detaches it
 * to allow it to run independently.
 * 
 * @return void
 */
void Seeder::start(){
    thread t(&Seeder::acceptConnections, this);
    t.detach();
}

/**
 * @brief Stops the Seeder by closing the socket.
 * 
 * This method closes the socket, stopping the Seeder from accepting further connections.
 * 
 * @return void
 */
void Seeder::stop(){
    m_seederSocket.closeSocket();
}

/**
 * @brief Accepts incoming connections and handles each connection in a separate thread.
 * 
 * This method runs in an infinite loop to accept incoming connections. For each connection,
 * it logs the event and spawns a new thread to handle the connection.
 * 
 * @return void
 */
void Seeder::acceptConnections(){
    while(true){
        try{
            int leecherSocketFd = m_seederSocket.acceptSocket();
            m_logger.log("INFO", "Connection established with FD of " + to_string(leecherSocketFd));
            thread t1(&Seeder::handleLeecher, this, leecherSocketFd);
            t1.detach();
        }
        catch(const string& e){
            m_logger.log("ERROR", e);
        }
    }
}

/**
 * @brief Handles communication with a connected leecher.
 * 
 * This method reads commands from the leecher, processes them, and sends back appropriate responses.
 * It handles two commands: "give_piece_info" and "give_piece".
 * 
 * @param leecherSocketFd The file descriptor for the connected leecher's socket.
 * 
 * @return void
 */
void Seeder::handleLeecher(int leecherSocketFd){
    while (true) {
        try{
            string receivedData = m_seederSocket.recvSocket(leecherSocketFd);

            if(receivedData == "") {
                m_logger.log("INFO", "LeecherSocket = " + to_string(leecherSocketFd) + " | Leecher closed the connection!!");
                break;
            }
            m_logger.log("COMMAND", "LeecherSocket = " + to_string(leecherSocketFd) + " | Recieved from leecher : " + receivedData);
            
            string response = "";

            try{
                response = "Success: " + executeCommand(receivedData, leecherSocketFd);
            }
            catch(const string& e){
                response = "Error: " + e;
            }
            
            m_seederSocket.sendSocket(leecherSocketFd, response);
        }
        catch(const string& e){
            m_logger.log("ERROR", "LeecherSocket = " + to_string(leecherSocketFd) + " | While handling leecher!! Error: " + e);
        }
    }
}

/**
 * @brief Executes a command received from a leecher.
 * 
 * This method parses and executes commands from the leecher. It supports the following commands:
 * - "give_piece_info": Returns available piece information for a given file and group.
 * - "give_piece": Returns the requested piece data for a given file, group, and piece number.
 * 
 * @param command The command string received from the leecher.
 * @param leecherSocketFd The file descriptor for the connected leecher's socket.
 * 
 * @return A string containing the result of the command execution or an error message.
 * 
 * @throws string If the command is invalid or if any errors occur during processing.
 */
string Seeder::executeCommand(string command, int leecherSocketFd){
    if(command == "") throw string("Invalid command!!");
    vector <string> tokens = Utils::tokenize(command, ' ');
    
    if(tokens.size() < 1) throw string("Invalid command!!");

    if(tokens[0] == "give_piece_info"){
        if(tokens.size() != 3) throw string("Invalid arguments to give_piece_info command!!");
        
        string fileName = tokens[1];
        string groupName = tokens[2];

        lock_guard <mutex> guard_1(Files::m_fileNameToFilePathMutex);
        lock_guard <mutex> guard_2(Files::m_filePathToAvailablePiecesMutex);

        // If {fileName, groupName} does not exist, return response with an empty string
        if(Files::m_fileNameToFilePath.find({fileName, groupName}) == Files::m_fileNameToFilePath.end()){
            return " ";
        }

        string filePath = Files::m_fileNameToFilePath[{fileName, groupName}];

        // If filePath does not exist, remove entry from the first map and return response with an empty string
        if(Files::m_filePathToAvailablePieces.find(filePath) == Files::m_filePathToAvailablePieces.end()){
            Files::m_fileNameToFilePath.erase({fileName, groupName});
            return " ";
        }

        // Building a space-separated response with available pieces
        string temp = "";
        for(auto it: Files::m_filePathToAvailablePieces[filePath]){
            temp.append(" " + to_string(it));
        }

        m_logger.log("INFO", "leecherSocket = " + to_string(leecherSocketFd) + 
            " | Sending response to leecher. Response = " + temp);
        return temp;
    }

    if(tokens[0] == "give_piece"){
        if(tokens.size() != 4) throw string("Invalid arguments to give_piece command!!");
        
        string fileName = tokens[1];
        string groupName = tokens[2];
        int pieceNumber = stoi(tokens[3]);

        lock_guard <mutex> guard_1(Files::m_fileNameToFilePathMutex);
        lock_guard <mutex> guard_2(Files::m_filePathToAvailablePiecesMutex);

        if(Files::m_fileNameToFilePath.find({fileName, groupName}) == Files::m_fileNameToFilePath.end()){
            throw string("File not Exist!!");
        }

        string filePath = Files::m_fileNameToFilePath[{fileName, groupName}];
        if(Files::m_filePathToAvailablePieces.find(filePath) == Files::m_filePathToAvailablePieces.end()){
            throw string("Filepieces map not Exist!!");
        }

        vector <int> tempVec = Files::m_filePathToAvailablePieces[filePath];
        if(find(tempVec.begin(), tempVec.end(), pieceNumber) == tempVec.end()){
            throw string("Piece not Found!!");
        }

        int fd = open(filePath.c_str(), O_RDONLY, S_IRUSR);
        if (fd == -1) {
            throw string("Failed to open file at Seeder!!");
        }

        int pieceOffset = PIECE_SIZE * pieceNumber;
        if(lseek(fd, pieceOffset, SEEK_SET) == -1){
            throw string("Failed to Seek at seeder!!");
        }

        char buffer[PIECE_SIZE];
        int bytesRead = read(fd, buffer, PIECE_SIZE);
        if(bytesRead == -1){
            throw string("Failed to Read a piece at seeder!!");
        }

        close(fd);
        
        string pieceData(buffer, bytesRead);

        m_logger.log("INFO", "leecherSocket = " + to_string(leecherSocketFd) + 
            " | Sending pieceData to leecher");

        return pieceData;

    }

    throw string("Invalid command!!");
}
