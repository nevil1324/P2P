#include "headers.h"

Logger generalLogger;

void handleQuitOfTracker(Tracker &tracker);

int main(int argc, char* argv[]){
    try{
        pair <string, int> trackerIpPort = Utils::processArgs(argc, argv);
        
        string trackerIp = trackerIpPort.first;
        int trackerPort = trackerIpPort.second;

        generalLogger = Logger(trackerIp, trackerPort, "general");

        generalLogger.log("INFO", "Creating tracker!!");
        
        // Tracker tracker(trackerIp, trackerPort);
        Tracker& tracker = Tracker::getInstance(trackerIp, trackerPort);
        generalLogger.log("INFO", "Tracker created successfully!!");   

        tracker.init();
        tracker.start();
        generalLogger.log("INFO", "Tracker started accepting connections!!");

        thread t(handleQuitOfTracker, ref(tracker));
        t.detach();

        while(1);
    }
    catch(const string& e){
        generalLogger.log("ERROR", "Creating tracker!! Error: " + e);
        cout << "Error: " + e + "\n" << flush;
        exit(1);
    }

    return 0;
}

void handleQuitOfTracker(Tracker &tracker){
    try{
        while(1){
            string s;
            cin >> s;
            if(s == "quit" || s == "exit"){
                tracker.stop();
                generalLogger.log("INFO" , "Tracker quit.");
                exit(0);
            }
        }
    }
    catch(const string& e){
        cout << string(RED) + "Error: " + e + "\n" + string(RESET) << flush;
        exit(1);
    }
}
