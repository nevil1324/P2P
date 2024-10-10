#include "../headers.h"

/**
 * @brief Initializes the Leecher instance by creating a client socket.
 */
void Leecher::init() {
    m_clientSocket.createSocket();
}

/**
 * @brief Connects to the tracker server.
 * 
 * @param trackerIp IP address of the tracker server.
 * @param trackerPort Port number of the tracker server.
 * 
 * @return void
 */
void Leecher::connectTracker(string trackerIp, int trackerPort) {
    m_clientSocket.connectSocket(trackerIp, trackerPort);
    m_clientSocket.setOptions();
    m_logger.log("SUCCESS", "Leecher connected to tracker at " + trackerIp + ":" + to_string(trackerPort) + ".");
}

/**
 * @brief Starts the Leecher instance by launching a detached thread to get user commands.
 * 
 * @return void
 */
void Leecher::start() {
    thread t(&Leecher::getCommand, this);
    t.detach();
}

/**
 * @brief Stops the Leecher instance by closing the client socket.
 * 
 * @return void
 */
void Leecher::stop() {
    m_clientSocket.closeSocket();
}

/**
 * @brief Continuously reads commands from the user and processes them.
 * 
 * @return void
 */
void Leecher::getCommand() {
    m_logger.log("INFO", "Started getting commands!!");
    while (true) {
        try {
            string inputFromClient;
            cout << ">> " << flush;
            getline(cin, inputFromClient);      
            processUserRequests(inputFromClient);
        } catch (const string& e) {
            cout << string(RED) + "Error: " + e + "\n" + string(RESET) << flush;
        }
    }
}

/**
 * @brief Processes user commands by delegating them to appropriate functions.
 * 
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::processUserRequests(string inputFromClient) {
    vector<string> tokens = Utils::tokenize(inputFromClient, ' ');
    
    if (tokens.empty()) return;

    if (tokens[0] == "quit" || tokens[0] == "exit") quit(tokens, inputFromClient);
    else if (tokens[0] == "create_user") createUser(tokens, inputFromClient);
    else if (tokens[0] == "login") login(tokens, inputFromClient);
    else if (tokens[0] == "create_group") createGroup(tokens, inputFromClient);
    else if (tokens[0] == "join_group") joinGroup(tokens, inputFromClient);
    else if (tokens[0] == "leave_group") leaveGroup(tokens, inputFromClient);
    else if (tokens[0] == "list_requests") listRequests(tokens, inputFromClient);
    else if (tokens[0] == "accept_request") acceptRequest(tokens, inputFromClient);
    else if (tokens[0] == "list_groups") listGroups(tokens, inputFromClient);
    else if (tokens[0] == "list_files") listFiles(tokens, inputFromClient);
    else if (tokens[0] == "upload_file") uploadFile(tokens, inputFromClient);
    else if (tokens[0] == "download_file") downloadFile(tokens, inputFromClient);
    else if (tokens[0] == "show_downloads") showDownloads(tokens, inputFromClient);
    else if (tokens[0] == "logout") logout(tokens, inputFromClient);
    else if (tokens[0] == "stop_share") stopShare(tokens, inputFromClient);
    else throw string("Invalid command!!");
}

/**
 * @brief Prints the response received from the tracker server based on the command.
 * 
 * @param tokens Command tokens.
 * @param response Response string from the tracker server.
 * 
 * @return void
 */
void Leecher::printResponse(vector<string> tokens, string response) {
    if (tokens[0] == "login") {
        vector<string> responseTokens = Utils::tokenize(response, ' ');
        response = "";
        for (int i = 0; i < (int)responseTokens.size(); i++) {
            if (i != 1) response.append(responseTokens[i] + " ");
        }
    }

    if (tokens[0] == "list_groups") {
        vector<string> responseTokens = Utils::tokenize(response, ' ');
        if (responseTokens.size() == 1) {
            cout << string(YELLOW) + "There is no group in the system!!\n" + string(RESET) << flush;
            return;
        }
        response = "List of groups is as follows : " + responseTokens[1];
        cout << response + "\n" << flush;
        return;
    }

    if (tokens[0] == "list_requests") {
        vector<string> responseTokens = Utils::tokenize(response, ' ');
        if (responseTokens.size() == 1) {
            cout << string(YELLOW) + "There is no pending joinee in the group!!\n" + string(RESET) << flush;
            return;
        }
        response = "List of pending requests in the group is as follows : " + responseTokens[1];
        cout << response + "\n" << flush;
        return;
    }

    if (tokens[0] == "list_files") {
        vector<string> responseTokens = Utils::tokenize(response, ' ');
        if (responseTokens.size() == 1) {
            cout << string(YELLOW) + "There are no files in the group!!\n" + string(RESET) << flush;
            return;
        }
        response = "List of files in the group is as follows : " + responseTokens[1];
        cout << response + "\n" << flush;
        return;
    }

    cout << string(GREEN) + response + "\n" + string(RESET) << flush;
}

/**
 * @brief Checks for errors in the response and throws an exception if an error is found.
 * 
 * @param response The response string from the tracker server.
 * 
 * @return void
 * 
 * @throws string Exception if error is found in the response.
 */
void Leecher::checkForError(string response) {
    if (response.substr(0, 5) == "Error") {
        throw response.substr(7);
    }
}

/**
 * @brief Sends a message to the tracker server and receives the response.
 * 
 * @param messageForTracker The message to be sent to the tracker server.
 * 
 * @return string The response received from the tracker server.
 */
string Leecher::sendTracker(string messageForTracker) {
    m_logger.log("COMMAND", "Sending to tracker : " + messageForTracker);
    
    m_clientSocket.sendSocket(messageForTracker);
    
    string response = m_clientSocket.recvSocket();
    m_logger.log("COMMAND", "Received from tracker : " + response);
    
    checkForError(response);
    
    return response;
}

/**
 * @brief Handles the quit command by logging out if necessary and stopping the Leecher instance.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::quit(vector<string> tokens, string inputFromClient) {
    if (m_authToken != "NULL") {
        try {
            m_logger.log("INFO", "authToken found!! Sending logout request to tracker");
            logout(tokens, "logout");
        } catch (const string& e) {
            m_logger.log("ERROR", "Error from tracker during 'quit logout'!! Error : " + e);    
        }
    } else {
        m_logger.log("INFO", "authToken not found!! No need to send logout request to tracker");
    }

    stop();
    m_logger.log("SUCCESS", "Leecher Quit.");
    exit(0);
}

/**
 * @brief Sends a user creation command to the tracker and prints the response.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::createUser(vector<string> tokens, string inputFromClient) {
    string messageForTracker = inputFromClient;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

/**
 * @brief Logs in the user and sends login information to the tracker.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::login(vector<string> tokens, string inputFromClient) {
    string messageForTracker = inputFromClient + " " + m_seederIp + ":" + to_string(m_seederPort);
    string response = sendTracker(messageForTracker);
    vector<string> responseTokens = Utils::tokenize(response, ' ');
    m_authToken = responseTokens[1];
    printResponse(tokens, response);
}

/**
 * @brief Creates a new group and sends the creation command to the tracker.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::createGroup(vector<string> tokens, string inputFromClient) {
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

/**
 * @brief Joins an existing group and sends the join command to the tracker.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::joinGroup(vector<string> tokens, string inputFromClient) {
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

/**
 * @brief Leaves a group and sends the leave command to the tracker.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::leaveGroup(vector<string> tokens, string inputFromClient) {
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

/**
 * @brief Lists all pending requests and sends the request command to the tracker.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::listRequests(vector<string> tokens, string inputFromClient) {
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

/**
 * @brief Accepts a request from a user and sends the accept command to the tracker.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::acceptRequest(vector<string> tokens, string inputFromClient) {
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

/**
 * @brief Lists all groups and sends the list groups command to the tracker.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::listGroups(vector<string> tokens, string inputFromClient) {
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

/**
 * @brief Lists all files in a group and sends the list files command to the tracker.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::listFiles(vector<string> tokens, string inputFromClient) {
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

/**
 * @brief Uploads a file to the tracker and sends the upload file command.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::uploadFile(vector<string> tokens, string inputFromClient) {
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

/**
 * @brief Downloads a file from the tracker and sends the download file command.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::downloadFile(vector<string> tokens, string inputFromClient) {
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

/**
 * @brief Shows all currently downloaded files and sends the show downloads command.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::showDownloads(vector<string> tokens, string inputFromClient) {
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

/**
 * @brief Logs out the user and sends the logout command to the tracker.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::logout(vector<string> tokens, string inputFromClient) {
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    m_authToken = "NULL";
    printResponse(tokens, response);
}

/**
 * @brief Stops sharing a file and sends the stop share command to the tracker.
 * 
 * @param tokens Command tokens.
 * @param inputFromClient The input command string from the user.
 * 
 * @return void
 */
void Leecher::stopShare(vector<string> tokens, string inputFromClient) {
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}
