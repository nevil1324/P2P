#include "../headers.h"

//: Declaration
// class Utils {
//     friend class Users;   
//     friend class Groups;  

//     private:
//         string generateToken(string payload);
//         string validateToken(string token);

//     public:
//         pair<string, int> processArgs(int argc, char* argv[]);
//         vector<string> tokenize(string buffer, char separator);
// };

pair <string, int> Utils::processArgs(int argc, char *argv[]){
    if(argc != 3){
        throw string("Invalid arguments!!");
    }

    char *trackerInfoFileName = argv[1];
    int trackerNumber = atoi(argv[2]);
    if(trackerNumber <= 0) {
        throw string("Tracker number is invalid!!");
    }

    int fd = open(trackerInfoFileName, O_RDONLY);
    if(fd < 0) {
        string s = trackerInfoFileName;
        throw string("Opening " + s + " file!!");
    }

    char buffer[524288];
    int bytesRead = read(fd, buffer, sizeof(buffer));
    if(bytesRead <= 0){
        string s = trackerInfoFileName;
        throw string("Reading " + s + " file!!");
    }

    vector <string> ipAndPorts = tokenize(buffer, '\n');
    if((int)ipAndPorts.size() < trackerNumber) {
        throw string("IP and port of tracker number " + to_string(trackerNumber) + " is not defined in file!!");
    }

    string ipAndPort = ipAndPorts[trackerNumber-1];

    vector <string> temp = tokenize(ipAndPort, ':');
    if((int)temp.size() != 2){
        throw string("Invalid format of ip:port of tracker number " + to_string(trackerNumber) + "!!");
    }

    string trackerIp = temp[0];
    int trackerPort = stoi(temp[1]);
    return {trackerIp, trackerPort};
}

vector <string> Utils::tokenize(string buffer, char separator){
    vector <string> ans;
    string temp;
    for(auto it: buffer){
        if(it == separator) {
            if(temp.size()) ans.push_back(temp);
            temp.clear();
        }
        else temp.push_back(it);
    }
    if(temp.size()) ans.push_back(temp);
    return ans;
}

string Utils::generateToken(string payload) {
    string secret_key = SECRET_KEY;
    // Get current time and calculate expiry time
    time_t currentTime = time(nullptr);
    time_t expiryTime = currentTime + TOKEN_EXPIRY_DURATION;
    
    string message = payload + ":" + to_string(expiryTime);

    // Generate HMAC-SHA256 signature
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), secret_key.c_str(), secret_key.length(),
                  (unsigned char*)message.c_str(), message.length(), nullptr, nullptr);

    // Convert to hexadecimal string
    char mdString[SHA256_DIGEST_LENGTH*2+1];
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);

    // Return the token: payload:expiry_time:signature
    return message + ":" + string(mdString);
}

string Utils::validateToken(string token) {
    string secret_key = SECRET_KEY;
    vector <string> tokens = tokenize(token, ':');
    if(tokens.size() != 3) throw string("Authentication failed!! Invalid token!!");
    
    string payload = tokens[0];
    time_t expiryTime = stol(tokens[1]);
    string signature = tokens[2];
    string payloadExpiry = payload + ":" + to_string(expiryTime);

    // Regenerate the HMAC-SHA256 signature
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), secret_key.c_str(), secret_key.length(),
                  (unsigned char*)payloadExpiry.c_str(), payloadExpiry.length(), nullptr, nullptr);

    // Convert to hexadecimal string
    char mdString[SHA256_DIGEST_LENGTH*2+1];
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);

    // Validate the signature
    if(signature != string(mdString)) throw string("Authentication failed!! Invalid signature!!");

    // Check if the token is expired
    time_t currentTime = time(nullptr);
    if (currentTime > expiryTime) throw string("Authentication failed!! Token expired!!");

    return payload;
}