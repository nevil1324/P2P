#include "../headers.h"

//: Declaration
// class Groups {
//     private:
//         Groups() = default;  
//         ~Groups() = default; 

//         Groups(const Groups&) = delete;
//         Groups& operator=(const Groups&) = delete;

//         static Groups* instance;
//         mutex m_groupsMutex;

//         unordered_map<string, Group> m_groups;

//     public:
//         static Groups& getInstance() {
//             static Groups instance;
//             return instance;
//         }

//         string addGroup(string groupName, string authToken);
//         string joinGroup(string groupName, string authToken);
//         string listRequests(string groupName, string authToken);
//         string listGroups(string authToken);
//         string acceptRequest(string groupName, string pendingUserName, string authToken);
//         string listFiles(string groupName, string authToken);
//         string uploadFile(string fileName, string groupName, string fileSize, string SHAs, string authToken);
//         string downloadFile(string fileName, string groupName, string authToken);
//         string stopShare(string groupName, string fileName, string authToken);
//         string leaveGroup(string groupName, string authToken);
// };

string Groups::addGroup(string groupName, string authToken){
    //: Validate that user is logged in
    string userName = Utils::validateToken(authToken);
    {
        //: Ensure that group with same name not exist
        lock_guard <mutex> guard(m_groupsMutex);
        if(m_groups.count(groupName)) {
            throw string("Group already exist!!");
        }

        //: Create new "Group" and add it to Groups.m_groups[] map
        Group group(groupName, {userName});
        m_groups[groupName] = group;
        return "Group created successfully!!";
    }
}

string Groups::joinGroup(string groupName, string authToken){
    //: Validate that user is logged in
    string userName = Utils::validateToken(authToken);
    {
        lock_guard <mutex> guard(m_groupsMutex);

        //: Ensure that group exist
        if(!m_groups.count(groupName)) {
            throw string("Group not exist!!");
        }

        Group& group = m_groups[groupName];

        //: Ensure that user is not a member of this group 
        if(count(group.m_participants.begin(), group.m_participants.end(), userName)){
            throw string("You are already a member of this group!!");
        }

        //: Ensure that joining request not already exist in group
        if(group.m_pendingJoins.count(userName)){
            throw string("Joining request has already been sent!!");
        }

        //: Add userName to pending joining request set
        group.m_pendingJoins.insert(userName);
        
        return "Joining request has been raised!!";
    }
}

string Groups::listRequests(string groupName, string authToken){
    //: Validate that user is logged in
    string userName = Utils::validateToken(authToken);
    {
        lock_guard <mutex> guard(m_groupsMutex);
        
        //: Ensure that group exist
        if(!m_groups.count(groupName)) {
            throw string("Group not exist!!");
        }

        Group& group = m_groups[groupName];
        
        //: Ensure that user is a admin of this group
        if(group.m_participants[0] != userName){
            throw string("You are not the admin of this group!!");
        }

        //: Building '\n' separated response
        string temp = "";
        for(auto it: group.m_pendingJoins) temp.append("\n" + it);
        return temp;
    }
}

string Groups::listGroups(string authToken){
    //: Validate that user is logged in
    string userName = Utils::validateToken(authToken);
    {
        lock_guard <mutex> guard(m_groupsMutex);

        //: Building '\n' separated response
        string temp = "";
        for(auto it: m_groups) temp.append("\n" + it.first);
        return temp;
    }

}

string Groups::acceptRequest(string groupName, string pendingUserName, string authToken){
    //: Validate that user is logged in
    string userName = Utils::validateToken(authToken);
    {
        lock_guard <mutex> guard(m_groupsMutex);

        //: Ensure that group exist
        if(!m_groups.count(groupName)) {
            throw string("Group not exist!!");
        }
        
        Group& group = m_groups[groupName];

        //: Ensure that user is a admin of this group
        if(group.m_participants[0] != userName){
            throw string("You are not an admin of the group!!");
        }

        //: Ensure that pending user exist in pending joining request set
        if(!group.m_pendingJoins.count(pendingUserName)){
            throw string("Username not exist in a pending requests list!!");
        }

        //: Remove pending user from pending joining request set
        group.m_pendingJoins.erase(pendingUserName);
        //: Add pending user to participants vector
        group.m_participants.push_back(pendingUserName);

        return "Member added to the group!!";
    }
}

string Groups::listFiles(string groupName, string authToken){
    //: Validate that user is logged in
    string userName = Utils::validateToken(authToken);
    {
        lock_guard <mutex> guard(m_groupsMutex);

        //: Ensure that group exist
        if(!m_groups.count(groupName)) {
            throw string("Group not exist!!");
        }

        Group& group = m_groups[groupName];
        
        //: Ensure that user is a member of this group
        if(!count(group.m_participants.begin(), group.m_participants.end(), userName)){
            throw string("You are not a member of this group!!");
        }

        //: Building '\n' separated response
        string temp = "";
        
        //: Iterate through all files exist in group
        for(auto& it: group.m_files){
            //: Here it.first = fileName & it.second = File object

            //: Iterate through all users that are sharing this perticular file
            for(auto userName: it.second.m_userNames){
                
                //: Ensure that user sharing this file has active session
                if(Users::m_userToIp.count(userName)){
                    //: If there is atleast one active user, 
                    
                    //: Add that file in response
                    temp.append("\n" + it.first);
                    
                    //: Check for next file
                    break;
                }
            }
        }

        return temp;
    }
}

string Groups::uploadFile(string fileName, string groupName, string fileSize, string SHAs, string authToken){
    //: Validate that user is logged in
    string userName = Utils::validateToken(authToken);
    {
        lock_guard <mutex> guard(m_groupsMutex);

        //: Ensure that group exist
        if(!m_groups.count(groupName)) {
            throw string("Group not exist!!");
        }

        Group& group = m_groups[groupName];
        
        //: Ensure that user is a member of this group
        if(!count(group.m_participants.begin(), group.m_participants.end(), userName)){
            throw string("You are not a member of this group!!");
        }

        vector <string> SHAVector = Utils::tokenize(SHAs, ':');

        //: Finding expected size of SHA vector
        int sizeOfSHAVector = (stoi(fileSize) / PIECE_SIZE) + 1; //: +1 for entire file's SHA at beggining of the vector
        if(stoi(fileSize) % PIECE_SIZE) sizeOfSHAVector++;

        //: Ensure that actual SHA vector size and expected SHA vector size are equal
        if((int)SHAVector.size() != sizeOfSHAVector) {
            throw string("Invalid (More/Less) number of SHAs!!");
        }

        //: If file already exist in group Check SHA
        if(group.m_files.count(fileName)){
            //: Ensure that SHA is matching
            if(group.m_files[fileName].m_SHA[0] != SHAVector[0]){
                throw string("File with same name but different content exist, change name of the file!!");
            }

            //: Insert userName into set
            group.m_files[fileName].m_userNames.insert(userName);
            return "File uploaded successfully!!";
        }

        //: If file not exist in group, Create new "File" and add it to the "Group.m_file[]" map
        File newFile(fileName, SHAVector, stoi(fileSize), {userName});
        group.m_files[fileName] = newFile;
        
        return "File uploaded successfully!!";
    }
}

string Groups::downloadFile(string fileName, string groupName, string authToken) {
    //: Validate that user is logged in
    string userName = Utils::validateToken(authToken);
    {
        lock_guard <mutex> guard(m_groupsMutex);

        //: Ensure that group exist
        if(!m_groups.count(groupName)) {
            throw string("Group not exist!!");
        }

        Group& group = m_groups[groupName];
        
        //: Ensure that user is a member of this group
        if(!count(group.m_participants.begin(), group.m_participants.end(), userName)){
            throw string("You are not a member of this group!!");
        }
       
       //: Ensure that demanded file exist in a group
        if(!group.m_files.count(fileName)){
            throw string("File not found!!");
        }

        File& file = group.m_files[fileName];
        
        //: Building a response in a formate of 
        //: "FileSize <space> FileSHA:Piece1SHA:Piece2SHA:...:PieceNSHA <space> IP:Port_1,IP:Port_2,...,IP:Port_N"
        string temp = "";

        //: Adding fileSize to response
        temp.append(to_string(file.m_size) + " ");

        //: Adding SHAs to response
        for(auto it : file.m_SHA) {
            temp.append(it + ":");
        }
        temp.push_back(' ');

        //: Adding IP:Ports to response
        {
            lock_guard <mutex> guard(Users::m_userToIpMutex);

            //: Building a string of IP:Port of active users that are currently sharing this file
            string activeUsers = "";
            for(auto it : file.m_userNames) {
                //: Append IP:Port of user if it has active session
                if(Users::m_userToIp.count(it)) {
                    activeUsers.append(Users::m_userToIp[it] + ",");
                }
            }
            //: Ensure that there is atleast one active user sharing this file
            if(activeUsers == "") throw string("There is no active user sharing this file as of now!!");
            temp += activeUsers;
        }
        return temp;
    }
}

string Groups::stopShare(string groupName, string fileName, string authToken) {
    //: Validate that user is logged in
    string userName = Utils::validateToken(authToken);
    {
        lock_guard <mutex> guard(m_groupsMutex);

        //: Ensure that group exist
        if(!m_groups.count(groupName)) {
            throw string("Group not exist!!");
        }

        Group& group = m_groups[groupName];

        //: Ensure that user is a member of this group
        if(!count(group.m_participants.begin(), group.m_participants.end(), userName)){
            throw string("You are not a member of this group!!");
        }

        //: Ensure that file exist in a group
        if(!group.m_files.count(fileName)){
            throw string("File doesn't exist!!");
        }

        File& file = group.m_files[fileName];

        //: Ensure that user sent a request is sharing this file
        if(!file.m_userNames.count(userName)){
            throw string("You are not sharing this file!!");
        }

        //: Remove userName from set
        file.m_userNames.erase(userName);

        //: Remove file from group if it does not have any user sharing
        if(file.m_userNames.empty()) {
            group.m_files.erase(fileName);
        }

        return "File Sharing is stopped!!";
    }
}

string Groups::leaveGroup(string groupName, string authToken) {
    //: Validate that user is logged in
    string userName = Utils::validateToken(authToken);
    {
        lock_guard <mutex> guard(m_groupsMutex);

        //: Ensure that group exist
        if(!m_groups.count(groupName)) {
            throw string("Group not exist!!");
        }

        Group& group = m_groups[groupName];
        
        //: Ensure that user is a member of this group
        if(!count(group.m_participants.begin(), group.m_participants.end(), userName)){
            throw string("You are not a member of this group!!");
        }

        //: Remove username from participants vector
        auto it = find(group.m_participants.begin(), group.m_participants.end(), userName);
        group.m_participants.erase(it);

        //: If Only 1 participant is there, delete this group from system
        if(m_groups[groupName].m_participants.size() == 0) {
            m_groups.erase(groupName);
            return "You left the group successfully!!";
        }

        //: Remove user from all files he is sharing
        for(auto &it : m_groups[groupName].m_files) {
            it.second.m_userNames.erase(userName);

            //: Remove file from group if it does not have any user sharing
            if(it.second.m_userNames.empty()) {
                m_groups[groupName].m_files.erase(it.first);
            }
        }
        return "You left the group successfully!!";
    }
}