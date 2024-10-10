#include "../headers.h"

/**
* @brief Creates a socket using IPv4 and TCP.
* @throws string If socket creation fails.
*/
void ClientSocket::createSocket(){
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
* @brief Connects to the server using the provided IP and port.
* @param serverIp The IP address of the server.
* @param serverPort The port number of the server.
* @throws string If the socket is not created, or if connecting to the server fails.
*/
void ClientSocket::connectSocket(string serverIp, int serverPort){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; ///< Address family for IPv4.
    serverAddr.sin_port = htons(serverPort); ///< Port number in network byte order.

    // Convert server IP address from string to binary form
    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr) <= 0) {
        throw string("Converting IP address " + serverIp + "!!\nError:" + string(strerror(errno)));
        closeSocket();
    }

    // Connect to the server
    if (connect(m_socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        throw string("Connecting to " + serverIp + ":" + to_string(serverPort) + "!!\nError: " + string(strerror(errno)));
        closeSocket();
    }
    m_serverIp = serverIp;
    m_serverPort = serverPort;
}

/**
* @brief Sets socket options such as address and port reuse.
* @throws string If setting socket options fails.
*/
void ClientSocket::setOptions(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    int opt = 1;
    if (setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        closeSocket();
        throw string("At setOptions!!\nError: " + string(strerror(errno)));
    }

    // Uncomment the following lines to set a receive timeout option
    // struct timeval timeout;      
    // timeout.tv_sec = 15;  // 15 seconds
    // timeout.tv_usec = 0; // 0 microseconds
    // setsockopt(m_socketFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
}

/**
* @brief Sends a message to the connected server.
* @param message The message to be sent to the server.
* @throws string If the socket is not created or if sending the message fails.
*/
void ClientSocket::sendSocket(string message){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    if(m_serverIp == "" || m_serverPort == -1){
        throw string("Socket is not connected with server!! Connect it first using connectSocket(string serverIp, int serverPort)!!");
    }
    string messageLength = to_string(message.size());
    message = messageLength + " " + message;

    // Send the message to the server
    if(send(m_socketFd, message.c_str(), message.size(), 0) < 0){
        close(m_socketFd);
        throw string("Sending message\nError: " + string(strerror(errno)));
    }
}

/**
* @brief Receives a message from the connected server.
* @return The received message as a string.
* @throws string If the socket is not created, if the socket is not connected to a server, 
*                 or if receiving the message fails.
*/
string ClientSocket::recvSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    if(m_serverIp == "" || m_serverPort == -1){
        throw string("Socket is not connected with server!! Connect it first using connectSocket(string serverIp, int serverPort)!!");
    }
    int totalDataLength = -1;
    string receivedData = "";

    while(true){
        char buffer[524288];
        int bytesRead = recv(m_socketFd, buffer, sizeof(buffer), 0);
        if(bytesRead == 0) {
            throw string("Error: Server closed the connection!!");
        }
        if (bytesRead < 0) {
            close(m_socketFd);
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                throw string("Receive timeout occurred!! No data received within 15 seconds!!");
            }
            throw string("Receiving data from server!!\nError: " + string(strerror(errno)));
        }

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

/**
* @brief Closes the client socket and resets internal state.
* @throws string If the socket is not created before attempting to close.
*/
void ClientSocket::closeSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    close(m_socketFd);
    m_socketFd = -1;
    m_serverIp = "";
    m_serverPort = -1;
}
