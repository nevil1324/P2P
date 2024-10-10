#include "../headers.h"

/**
* @brief Creates a socket using IPv4 and TCP.
* @throws string If socket creation fails.
*/
void ServerSocket::createSocket(){
    int domainAddressFormat = AF_INET; ///< Address family for IPv4.
    int type = SOCK_STREAM; ///< Socket type for TCP.
    int protocol = 0; ///< Protocol for TCP/IP.

    // Create socket and assign file descriptor to m_socketFd
    m_socketFd = socket(domainAddressFormat, type, protocol);
    if(m_socketFd < 0){
        throw string("Creating a socket!!\nError: " + string(strerror(errno)));
    }
}

/**
* @brief Sets socket options to allow address and port reuse.
* @throws string If setting socket options fails.
*/
void ServerSocket::setOptions(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    int opt = 1;
    if (setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        closeSocket();
        throw string("At setOptions!!\nError: " + string(strerror(errno)));
    }
}

/**
* @brief Binds the socket to the server IP and port.
* @throws string If binding fails.
*/
void ServerSocket::bindSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; ///< Address family for IPv4.
    serverAddr.sin_port = htons(m_serverPort); ///< Port number in network byte order.
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); ///< Bind to any available IP address.

    // Bind socket to the specified address and port
    if (bind(m_socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        closeSocket();
        throw string("Binding socket!!\nError: " + string(strerror(errno)));
    }
}

/**
* @brief Listens for incoming connections on the bound socket.
* @throws string If listening fails.
*/
void ServerSocket::listenSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }

    // Listen for incoming connections, allowing a backlog of 5000 connections
    if (listen(m_socketFd, 5000) < 0) {
        closeSocket();
        throw string("Listening socket!!\nError: " + string(strerror(errno)));
    }
}

/**
* @brief Accepts an incoming connection request.
* @return The file descriptor for the accepted client socket.
* @throws string If accepting connection fails.
*/
int ServerSocket::acceptSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }

    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    // Accept an incoming connection
    int clientSocket = accept(m_socketFd, (struct sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket < 0) {
        close(clientSocket);
        throw string("Accepting connection!!\nError:" + string(strerror(errno)));
    }
    return clientSocket;
}

/**
* @brief Closes the server socket and resets internal state.
* @throws string If socket is not created before attempting to close.
*/
void ServerSocket::closeSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    close(m_socketFd);
    m_socketFd = -1;
    m_serverIp = "";
    m_serverPort = -1;
}

/**
* @brief Sends a response message to a client socket.
* @param clientSocketFd The file descriptor of the client socket.
* @param response The response message to be sent.
* @throws string If sending message fails.
*/
void ServerSocket::sendSocket(int clientSocketFd, string response){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    string messageLength = to_string(response.size());
    response = messageLength + " " + response;

    // Send response to client socket
    if(send(clientSocketFd, response.c_str(), response.size(), 0) < 0){
        close(clientSocketFd);
        throw string("Sending message to client at fd " + to_string(clientSocketFd) + "!!\nError: " + string(strerror(errno)));
    }
}

/**
* @brief Receives a message from a client socket.
* @param clientSocketFd The file descriptor of the client socket.
* @return The received message as a string.
* @throws string If receiving message fails.
*/
string ServerSocket::recvSocket(int clientSocketFd){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    int totalDataLength = -1;
    string receivedData = "";

    while(true){
        char buffer[524288]; // 512 KB Buffer
        int bytesRead = recv(clientSocketFd, buffer, sizeof(buffer), 0);

        if(bytesRead == 0) return ""; // Connection closed by client

        if (bytesRead < 0) {
            close(clientSocketFd);
            throw string("Error receiving message from client-socket at fd " + to_string(clientSocketFd) + "!! Connection closed forcefully!!");
        }

        //: First token of receivedData will always be a length of total data to be recieved
        receivedData += string(buffer, bytesRead);

        if(totalDataLength == -1) {
            vector<string> temp = Utils::tokenize(receivedData, ' ');
            totalDataLength = stoi(temp[0]) - (bytesRead - (temp[0].size() + 1));
            receivedData = receivedData.substr(temp[0].size() + 1);
        }
        else {
            totalDataLength -= bytesRead;
        }

        if(totalDataLength == 0) {
            break;
        }
    }

    return receivedData;
}
