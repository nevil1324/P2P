void ServerSocket::createSocket(){
    int domainAddressFormat = AF_INET; 
    int type = SOCK_STREAM; 
    int protocol = 0;
    
    m_socketFd = socket(domainAddressFormat, type, protocol);
    if(m_socketFd < 0){
        throw string("Creating a socket!!\nError: " + string(strerror(errno))) ;
    }

}

void ServerSocket::setOptions(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    int opt = 1;
    if (setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        closeSocket();
        throw string("At setOptions!!\nError: " + string(strerror(errno))) ;
    }
}

void ServerSocket::bindSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; 
    serverAddr.sin_port = htons(m_serverPort); 
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); 

    if (bind(m_socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        closeSocket();
        throw string("Binding socket!!\nError: " + string(strerror(errno)));
    }

}

void ServerSocket::listenSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }

    if (listen(m_socketFd, 5000) < 0) {
        closeSocket();
        throw string("Listening socket!!\nError: " + string(strerror(errno)));
    }
}

int ServerSocket::acceptSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    int clientSocket = accept(m_socketFd, (struct sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket < 0) {
        close(clientSocket);
        throw string("Accepting connection!!\nError:" + string(strerror(errno)));
    }
    return clientSocket;   
}

void ServerSocket::closeSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    close(m_socketFd);
    m_socketFd = -1;
    m_serverIp = "";
    m_serverPort = -1;
}

void ServerSocket::sendSocket(int clientSocketFd, string response){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    string messageLength = to_string(response.size());
    response = messageLength + " " + response;

    if(send(clientSocketFd, response.c_str(), response.size(), 0) < 0){
        close(clientSocketFd);
        throw string("Sending message to client at fd " + to_string(clientSocketFd) + "!!\nError: " + string(strerror(errno)));
    }
}

string ServerSocket::recvSocket(int clientSocketFd){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    int totalDataLength = -1;
    string receivedData = "";

    while(true){
        char buffer[524288];
        int bytesRead = recv(clientSocketFd, buffer, sizeof(buffer), 0);

        if(bytesRead == 0) return "";

        if (bytesRead < 0) {
            close(clientSocketFd);
            throw string("Error recieving message from client-socket at fd " + to_string(clientSocketFd) + "!! Connection closed forcefully!!");
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

