#include "../headers.h"

//: Declaration
// class Users {
//     friend class Groups;    

//     private:
//         Users() = default;      
//         ~Users() = default;     

//         Users(const Users&) = delete;
//         Users& operator=(const Users&) = delete;

//         static Users* instance;

//         mutex m_usersMutex;
//         mutex m_userToIpMutex;

//         unordered_map<string, User> m_users;
//         unordered_map<string, string> m_userToIp;

//     public:
//         static Users& getInstance() {
//             static Users instance;
//             return instance;
//         }

//         string addUser(string userName, string password);
//         string loginUser(string userName, string password, string seederIpPort);
//         string logoutUser(string authToken);
// };

mutex Users::m_userToIpMutex;
unordered_map<string, string> Users::m_userToIp;

string Users::addUser(string userName, string password){
    lock_guard <mutex> guard(m_usersMutex);

    //: Ensure that username not exists
    if(m_users.count(userName)) {
        throw string("Username already exist!!");
    }

    //: Create new "User" and add it to Users.m_users[] map
    m_users[userName] = User(userName, password);
    return "User Registered Sucessfully!!";
}

string Users::loginUser(string userName, string password, string seederIpPort){
    {
        lock_guard <mutex> guard(m_usersMutex);
        //: Ensure that username exist
        if(!m_users.count(userName)) {
            throw string("Username not found!!");
        }
        //: Validate the password
        if(m_users[userName].m_password != password) {
            throw string("Incorrect password!!");
        }
    }

    //: Saving IP & Port of loggedin User
    {
        lock_guard <mutex> guard(m_userToIpMutex);
        
        //: Ensure that user not already logged in from other place
        if(m_userToIp.count(userName)) {
            throw string("Session already exist!! Logout from current session first!!");
        }

        //: Add IP:Port in Users.m_userToIp[] map
        m_userToIp[userName] = seederIpPort;
    }
    //: Generate authToken
    string payload = userName;
    string authToken = Utils::generateToken(payload);
    
    return authToken + " " + "User logged in Succesfully!!";
}

string Users::logoutUser(string authToken){
    //: Validate that user is logged in
    string userName = Utils::validateToken(authToken);

    //: Remove entry of {userName, IP:Port} form "Users.m_userToIp"
    lock_guard <mutex> guard(m_userToIpMutex);
    m_userToIp.erase(userName);

    return string("User logged out successfully!!");
}