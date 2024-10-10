#include "../headers.h"

void Tracker::init(){
    m_trackerSocket.createSocket();
    m_trackerSocket.setOptions();
    m_trackerSocket.bindSocket();
    m_trackerSocket.listenSocket();
    cout << "Tracker started listening!!\n" << flush;
    m_logger.log("Success", "Tracker started listening!!");
}

void Tracker::start(){
    thread t(&Tracker::acceptConnections, this);
    t.detach();
}

void Tracker::stop(){
    m_trackerSocket.closeSocket();
}

void Tracker::acceptConnections(){
    while(true){
        try{
            int leecherSocketFd = m_trackerSocket.acceptSocket();
            cout << "Connection established with FD of " + to_string(leecherSocketFd) + "\n" << flush;
            m_logger.log("INFO", "Connection established with FD of " + to_string(leecherSocketFd));
            
            thread t1(&Tracker::handleLeecher, this, leecherSocketFd);
            t1.detach();
        }
        catch(const string& e){
            m_logger.log("ERROR", e);
        }
    }
}

void Tracker::handleLeecher(int leecherSocketFd){
    while (true) {
        try{
            string receivedData = m_trackerSocket.recvSocket(leecherSocketFd);

            if(receivedData == "") {
                m_logger.log("INFO", "LeecherSocket = " + to_string(leecherSocketFd) + " | Leecher closed the connection!!");
                break;
            }
            m_logger.log("COMMAND", "LeecherSocket = " + to_string(leecherSocketFd) + " | Recieved from leecher : " + receivedData);
            
            string response = "";
            try{
                response = "Success: " + executeCommand(receivedData);
            }
            catch(const string& e){
                response = "Error: " + e;
            }
            
            m_trackerSocket.sendSocket(leecherSocketFd, response);
        }
        catch(const string& e){
            m_logger.log("ERROR", "LeecherSocket = " + to_string(leecherSocketFd) + " | While handling leecher!! Error: " + e);
        }
    }
}

string Tracker::executeCommand(string command){
    if(command == "") throw string("Invalid command!!");
    vector <string> tokens = Utils::tokenize(command, ' ');
    
    if(tokens.size() < 1) throw string("Invalid command!!");

    if(tokens[0] == "create_user"){
        if(tokens.size() != 3) throw string("Invalid arguments to create_user command!!");
        
        string userName = tokens[1];
        string password = tokens[2];

        return m_users.addUser(userName, password);
    }

    if(tokens[0] == "login"){
        if(tokens.size() != 4) throw string("Invalid arguments to login command!!");
        
        string userName = tokens[1];
        string password = tokens[2];
        string seederIpPort = tokens[3];

        return m_users.loginUser(userName, password, seederIpPort);
    }

    if(tokens[0] == "create_group") {
        if(tokens.size() != 3) throw string("Invalid arguments to create_group command!!");
        string groupName = tokens[1];
        string authToken = tokens[2];
        return m_groups.addGroup(groupName, authToken);
    }

    if(tokens[0] == "join_group"){
        if(tokens.size() != 3) throw string("Invalid arguments to join_group command!!");
        string groupName = tokens[1];
        string authToken = tokens[2];
        return m_groups.joinGroup(groupName, authToken);
    }

    if(tokens[0] == "list_requests"){
        if(tokens.size() != 3) throw string("Invalid arguments to list_requests command!!");
        string groupName = tokens[1];
        string authToken = tokens[2];
        return m_groups.listRequests(groupName, authToken);
    }

    if(tokens[0] == "list_groups"){
        if(tokens.size() != 2) throw string("Invalid arguments to list_groups command!!");
        string authToken = tokens[1];
        return m_groups.listGroups(authToken);
    }

    if(tokens[0] == "accept_request"){
        if(tokens.size() != 4) throw string("Invalid arguments to accept_request command!!");
        string groupName = tokens[1];
        string userName = tokens[2];
        string authToken = tokens[3];
        return m_groups.acceptRequest(groupName, userName, authToken);
    }

    if(tokens[0] == "list_files"){
        if(tokens.size() != 3) throw string("Invalid arguments to list_files command!!");
        string groupName = tokens[1];
        string authToken = tokens[2];
        return m_groups.listFiles(groupName, authToken);
    }

    if(tokens[0] == "upload_file"){
        if(tokens.size() != 6) throw string("Invalid arguments to upload_file command!!");
        string fileName = tokens[1];
        string groupName = tokens[2];
        string fileSize = tokens[3];
        string SHAs = tokens[4];
        string authToken = tokens[5];
        return m_groups.uploadFile(fileName, groupName, fileSize, SHAs, authToken);
    }

    if(tokens[0] == "download_file"){
        if(tokens.size() != 4) throw string("Invalid arguments to download_file command!!");
        string groupName = tokens[1];
        string fileName = tokens[2];
        string authToken = tokens[3];
        return m_groups.downloadFile(fileName, groupName, authToken);
    }

    if(tokens[0] == "stop_share"){
        if(tokens.size() != 4) throw string("Invalid arguments to stop_share command!!");
        string groupName = tokens[1];
        string fileName = tokens[2];
        string authToken = tokens[3];
        return m_groups.stopShare(groupName, fileName, authToken);
    }
    
    if(tokens[0] == "leave_group"){
        if(tokens.size() != 3) throw string("Invalid arguments to leave_group command!!");
        string groupName = tokens[1];
        string authToken = tokens[2];
        return m_groups.leaveGroup(groupName, authToken);
    }

    if(tokens[0] == "logout"){
        if(tokens.size() != 2) throw string("Invalid arguments to logout command!!");
        string authToken = tokens[1];
        return m_users.logoutUser(authToken);
    }

    throw string("Invalid command!!");
}


