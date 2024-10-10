#include "headers.h"

Logger generalLogger;

/**
 * @brief Function to start the Seeder in a separate thread.
 * 
 * This function initializes and starts the Seeder, logging its progress.
 * The function runs indefinitely, accepting connections.
 * 
 * @param seederIp The IP address of the Seeder.
 * @param seederPort The port number of the Seeder.
 */
void startSeeder(string seederIp, int seederPort);

int main(int argc, char* argv[]){
    try{
        // Process command-line arguments to extract IP addresses and port numbers
        vector <string> ipAndPorts = Utils::processArgs(argc, argv);

        // Check if the correct number of arguments have been provided
        if(ipAndPorts.size() != 4) {
            cout << string(RED) + "Args processing failed!!\n" + string(RESET) << flush;
            return 1; // Exit with an error code if arguments are incorrect
        }

        // Extract IP and port values from processed arguments
        string seederIp = ipAndPorts[0];
        int seederPort = stoi(ipAndPorts[1]);
        string trackerIp = ipAndPorts[2];
        int trackerPort = stoi(ipAndPorts[3]);

        // Initialize the logger with the Seeder's IP and port
        generalLogger = Logger(seederIp, seederPort, "general");

        // Start the Seeder in a separate detached thread
        thread t(startSeeder, seederIp, seederPort);
        t.detach(); // Detach the thread to allow it to run independently

        generalLogger.log("INFO", "Creating leecher!!");
        
        // Get the singleton instance of the Leecher and initialize it
        Leecher& leecher = Leecher::getInstance(seederIp, seederPort);
        generalLogger.log("INFO", "Leecher created successfully!!");   

        // Initialize and connect the Leecher to the tracker
        leecher.init();
        leecher.connectTracker(trackerIp, trackerPort);
        generalLogger.log("INFO", "Leecher connected to tracker successfully!!");
        
        // Start the Leecher
        leecher.start();
        generalLogger.log("INFO", "Leecher is ready for commands!!");

        // Keep the main thread running indefinitely
        while(1);
    }
    catch(const string& e){
        // Handle exceptions and log errors
        generalLogger.log("ERROR", "Creating leecher!! Error: " + e);
        cout << string(RED) + "Error: " + e + "\n" + string(RESET) << flush;
        exit(1); // Exit with an error code
    }
}

/**
 * @brief Function to start the Seeder process.
 * 
 * This function initializes and starts the Seeder, logging its progress.
 * The function runs indefinitely, accepting connections.
 * 
 * @param seederIp The IP address of the Seeder.
 * @param seederPort The port number of the Seeder.
 */
void startSeeder(string seederIp, int seederPort){
    try{
        generalLogger.log("INFO", "Creating seeder!!");
        
        // Get the singleton instance of the Seeder and initialize it
        Seeder& seeder = Seeder::getInstance(seederIp, seederPort);

        generalLogger.log("INFO", "Seeder created successfully!!");
        
        // Initialize and start the Seeder
        seeder.init();
        seeder.start();
        
        generalLogger.log("INFO", "Seeder started accepting connections!!");
        
        // Keep the Seeder thread running indefinitely
        while(1);
    }
    catch(const string& e){
        // Handle exceptions and log errors
        generalLogger.log("ERROR", "Creating seeder!! Error: " + e);
        cout << string(RED) + "Error: " + e + "\n" + string(RESET) << flush;
        exit(1); // Exit with an error code
    }
}
