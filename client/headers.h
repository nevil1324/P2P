#include <iostream>                 // For standard I/O operations
#include <string>                   // For string
#include <vector>                   // For vector
#include <map>                      // For map
#include <set>                      // For set
#include <queue>                    // For queue
#include <atomic>                   // For atomic
#include <condition_variable>       // For condition_variable
#include <thread>                   // For threads
#include <mutex>                    // For mutex
#include <functional>               // for function <void()>
#include <arpa/inet.h>              // For socket programming
#include <fcntl.h>                  // For open()
#include <unistd.h>                 // For close()
#include <sys/stat.h>               // For stat()
#include <errno.h>                  // For errno
#include <cstring>                  // For strerror
#include <openssl/hmac.h>           // For HMAC operations
#include <openssl/sha.h>            // For SHA hashing
#include <random>                   // For randomness at piece selection

#define POOL_SIZE 10
#define PIECE_SIZE 1024

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

using namespace std;

/**
 * @class Logger
 * @brief A class to handle logging messages to a file.
 *        It supports creating directories and log files, and writing log entries.
 */
class Logger {
    private:
        mutex m_logMutex; ///< Mutex to ensure thread-safe access to the log file.

        string m_seederIp; ///< IP address of the seeder.
        string m_seederPort; ///< Port number of the seeder.
        string m_logDirPath; ///< Directory path where logs are stored.
        string m_logFilePath; ///< File path for the log file.

    public:
        /**
        * @brief Default constructor for Logger.
        */
        Logger() = default;

        /**
        * @brief Constructs a Logger with specified IP, port, and log file name.
        * @param seederIp The IP address of the seeder.
        * @param seederPort The port number of the seeder.
        * @param name The name of the log file.
        */
        Logger(string seederIp, int seederPort, string name);

        /**
        * @brief Move constructor for Logger.
        * @param other The Logger object to move from.
        * @note Mutexes are not movable; hence, only data members are moved.
        */
        Logger(Logger&& other) noexcept;

        /**
        * @brief Move assignment operator for Logger.
        * @param other The Logger object to move from.
        * @return A reference to this Logger object.
        * @note Mutexes are not movable; hence, only data members are moved.
        */
        Logger& operator=(Logger&& other) noexcept;

        /**
        * @brief Logs a message to the log file.
        * @param type The type of the log message (e.g., "ERROR", "INFO").
        * @param content The content of the log message.
        * @details Adds a timestamp and type prefix to the log message before writing to the file.
        */
        void log(string type, string content);
};

/**
 * @class ThreadPool
 * @brief A thread pool class to manage a pool of worker threads and a queue of tasks.
 *        Tasks are executed by threads in the pool, allowing for concurrent task execution.
 */
class ThreadPool {
    private:
        vector<thread> m_workers; ///< Vector of worker threads in the pool.
        queue<function<void()>> m_tasks; ///< Queue of tasks to be executed by the worker threads.

        mutex m_queueMutex; ///< Mutex to protect access to the task queue.
        condition_variable m_condition; ///< Condition variable to notify worker threads about new tasks or stop signal.
        condition_variable m_waitCondition; ///< Condition variable to wait for all tasks to complete.

        atomic<bool> m_stop; ///< Flag indicating whether the thread pool should stop processing tasks.
        atomic<int> m_activeTasks; ///< Number of active tasks currently being processed.

        /**
        * @brief The worker thread function that continuously processes tasks from the queue.
        */
        void workerThread();

    public:
        /**
        * @brief Constructs a ThreadPool with a specified number of worker threads.
        * @param numThreads The number of worker threads to create.
        */
        ThreadPool(size_t numThreads);

        /**
        * @brief Destroys the ThreadPool and joins all worker threads.
        */
        ~ThreadPool();

        /**
        * @brief Enqueues a task for execution by the thread pool.
        * @param task The task to be executed. It is a callable object (function, lambda, etc.).
        * @throws runtime_error If the thread pool has been stopped and no more tasks can be enqueued.
        */
        void enqueueTask(function<void()> task);

        /**
        * @brief Blocks until all enqueued tasks have been completed.
        */
        void wait();
};

/**
 * @class ClientSocket
 * @brief A class that handles client-side socket operations including creating, connecting, 
 *        sending, receiving data, and closing the socket.
 */
class ClientSocket {
    private:
        string m_serverIp; ///< The IP address of the server to connect to.
        int m_serverPort{-1}; ///< The port number of the server.
        int m_socketFd{-1}; ///< File descriptor for the socket.

    public:
        /**
        * @brief Default constructor for ClientSocket.
        */
        ClientSocket() = default;

        /**
        * @brief Creates a socket and assigns it to m_socketFd.
        * @throws string If socket creation fails.
        */
        void createSocket();

        /**
        * @brief Sets socket options such as address and port reuse.
        * @throws string If setting socket options fails.
        */
        void setOptions();

        /**
        * @brief Connects to a server at the specified IP and port.
        * @param serverIp The IP address of the server.
        * @param serverPort The port number of the server.
        * @throws string If connecting to the server fails.
        */
        void connectSocket(string serverIp, int serverPort);

        /**
        * @brief Sends a message to the connected server.
        * @param message The message to be sent to the server.
        * @throws string If sending the message fails.
        */
        void sendSocket(string message);

        /**
        * @brief Receives a message from the connected server.
        * @return The received message as a string.
        * @throws string If receiving the message fails or if the connection is closed.
        */
        string recvSocket();

        /**
        * @brief Closes the client socket and resets internal state.
        * @throws string If socket is not created before attempting to close.
        */
        void closeSocket();
};

/**
 * @class ServerSocket
 * @brief A class that handles server-side socket operations including creating, binding, 
 *        listening, accepting connections, sending and receiving data.
 */
class ServerSocket {
    private:
        string m_serverIp; ///< The IP address of the server.
        int m_serverPort; ///< The port number for the server socket.
        int m_socketFd{-1}; ///< File descriptor for the socket.

    public:
        /**
        * @brief Default constructor for ServerSocket.
        */
        ServerSocket() = default;

        /**
        * @brief Parameterized constructor to initialize the server's IP and port.
        * @param serverIp The IP address of the server.
        * @param serverPort The port number on which the server will listen.
        */
        ServerSocket(string serverIp, int serverPort);

        /**
        * @brief Creates a socket and assigns it to m_socketFd.
        * @throws string If socket creation fails.
        */
        void createSocket();

        /**
        * @brief Sets socket options for reusing address and port.
        * @throws string If setting options fails.
        */
        void setOptions();

        /**
        * @brief Binds the socket to the server's IP and port.
        * @throws string If binding fails.
        */
        void bindSocket();

        /**
        * @brief Listens for incoming connections on the bound socket.
        * @throws string If listening fails.
        */
        void listenSocket();

        /**
        * @brief Accepts an incoming connection request.
        * @return The file descriptor for the accepted client socket.
        * @throws string If accepting connection fails.
        */
        int acceptSocket();

        /**
        * @brief Sends a response message to a client socket.
        * @param clientSocketFd The file descriptor of the client socket.
        * @param response The response message to be sent.
        * @throws string If sending message fails.
        */
        void sendSocket(int clientSocketFd, string response);

        /**
        * @brief Receives a message from a client socket.
        * @param clientSocketFd The file descriptor of the client socket.
        * @return The received message as a string.
        * @throws string If receiving message fails.
        */
        string recvSocket(int clientSocketFd);

        /**
        * @brief Closes the server socket and resets internal state.
        * @throws string If socket is not created before attempting to close.
        */
        void closeSocket();
};

/**
 * @class Utils
 * @brief A utility class for handling file operations, string processing, and SHA-256 hashing.
 * @details This class provides static methods for processing command-line arguments,
 *          tokenizing strings, computing SHA-256 hashes for files and pieces of data,
 *          and retrieving file sizes. The class is designed to be used without instantiation.
 */
class Utils {
    private:
        friend class Leecher;
        
        Utils() = delete; ///< Prevents instantiation of the Utils class.

        /**
        * @brief Computes the SHA-256 hashes of a file.
        * @param filePath The path to the file.
        * @return A vector of SHA-256 hashes, where the first entry is the hash of the entire file
        *         and the subsequent entries are hashes of individual pieces.
        * @throws string If there is an error opening or reading the file.
        */
        static vector<string> findSHA(string filePath);

        /**
        * @brief Computes the SHA-256 hash of a piece of data.
        * @param pieceData The data for which to compute the hash.
        * @return The SHA-256 hash as a hexadecimal string.
        */
        static string findPieceSHA(string pieceData);

        /**
        * @brief Retrieves the size of a file.
        * @param filePath The path to the file.
        * @return The size of the file in bytes.
        * @throws string If there is an error opening or seeking the file.
        */
        static int giveFileSize(string filePath);

    public:
        /**
        * @brief Processes command-line arguments.
        * @param argc The argument count.
        * @param argv The argument vector.
        * @return A vector of strings containing the processed arguments.
        * @throws string If the number of arguments is incorrect or if any argument is invalid.
        */
        static vector<string> processArgs(int argc, char *argv[]);

        /**
        * @brief Tokenizes a string based on a specified separator.
        * @param buffer The string to tokenize.
        * @param separator The character to use as the delimiter.
        * @return A vector of strings obtained by splitting the input string.
        */
        static vector<string> tokenize(string buffer, char separator);
};

/**
 * @class Files
 * @brief Manages file paths and available pieces for files in a shared context.
 * @details This class provides static methods to add and retrieve file paths and pieces,
 *          as well as check the availability of pieces. The class is designed to be used
 *          without instantiation.
 */
class Files {
    private:
        friend class Leecher;
        friend class Seeder;

        static mutex m_fileNameToFilePathMutex; ///< Mutex to protect access to fileNameToFilePath.
        static mutex m_filePathToAvailablePiecesMutex; ///< Mutex to protect access to filePathToAvailablePieces.

        static map<pair<string, string>, string> m_fileNameToFilePath; ///< Maps file name and group name to file path.
        static map<string, vector<int>> m_filePathToAvailablePieces; ///< Maps file path to a vector of available piece numbers.

        /**
        * @brief Adds a file path to the map.
        * @param fileName The name of the file.
        * @param groupName The name of the group.
        * @param filePath The path to the file.
        */
        static void addFilepath(string fileName, string groupName, string filePath);

        /**
        * @brief Adds a piece number to the list of available pieces for a given file path.
        * @param filePath The path to the file.
        * @param pieceNumber The piece number to add.
        */
        static void addPieceToFilepath(string filePath, int pieceNumber);

        /**
        * @brief Retrieves the file path for a given file name and group name.
        * @param fileName The name of the file.
        * @param groupName The name of the group.
        * @return The file path, or an empty string if not found.
        */
        static string giveFilePath(string fileName, string groupName);

        /**
        * @brief Retrieves a string of available pieces for a given file path.
        * @param filePath The path to the file.
        * @return A string of available piece numbers separated by spaces, or an empty string if none.
        */
        static string giveAvailablePieces(string filePath);

        /**
        * @brief Checks if a specific piece is available for a given file path.
        * @param filePath The path to the file.
        * @param pieceNumber The piece number to check.
        * @return True if the piece is available, false otherwise.
        */
        static bool isPieceAvailable(string filePath, int pieceNumber);

    public:
        Files() = default;
};

/**
 * @class Leecher
 * @brief Handles all user operations for a file-sharing system.
 * 
 * The Leecher class manages interactions with a tracker server and processes user 
 * commands related to file downloading and sharing. It maintains information about 
 * files being downloaded, downloaded successfully, and failed downloads. The class 
 * ensures thread-safe access to shared resources using mutexes and logs operations 
 * for debugging and tracking.
 */
class Leecher {
    private:
        mutex m_downloadFileMutex; ///< Mutex to synchronize access to download file operations.

        string m_authToken{"NULL"}; ///< Authentication token for the user.
        string m_seederIp; ///< IP address of the seeder.
        int m_seederPort; ///< Port number of the seeder.

        ClientSocket m_clientSocket; ///< Client socket for network communication.
        Logger m_logger; ///< Logger for tracking events and errors.

        set<pair<string, string>> m_downloadingFiles; ///< Set of files currently being downloaded (groupId, fileName).
        set<pair<string, string>> m_downloadedFiles; ///< Set of files that have been downloaded (groupId, fileName).
        set<pair<string, string>> m_downloadFailFiles; ///< Set of files that failed to download (groupId, fileName).

        /**
         * @brief Reads and processes commands from the user.
         */
        void getCommand();

        /**
         * @brief Processes user requests based on input commands.
         * @param inputFromClient The command string entered by the user.
         */
        void processUserRequests(string inputFromClient);

        /**
         * @brief Sends a message to the tracker and receives the response.
         * @param messageForTracker The message to be sent to the tracker.
         * @return The response received from the tracker.
         */
        string sendTracker(string messageForTracker);

        /**
         * @brief Checks for errors in the tracker response.
         * @param response The response received from the tracker.
         * @throws string If the response contains an error.
         */
        void checkForError(string response);

        /**
         * @brief Prints the response received from the tracker.
         * @param tokens Command tokens.
         * @param response The response to be printed.
         */
        void printResponse(vector<string> tokens, string response);

        // Command handling methods
        void quit(vector<string> tokens, string response);
        void createUser(vector<string> tokens, string inputFromClient);
        void login(vector<string> tokens, string inputFromClient);
        void createGroup(vector<string> tokens, string inputFromClient);
        void joinGroup(vector<string> tokens, string inputFromClient);
        void leaveGroup(vector<string> tokens, string inputFromClient);
        void listRequests(vector<string> tokens, string inputFromClient);
        void acceptRequest(vector<string> tokens, string inputFromClient);
        void listGroups(vector<string> tokens, string inputFromClient);
        void listFiles(vector<string> tokens, string inputFromClient);
        void uploadFile(vector<string> tokens, string inputFromClient);
        void downloadFile(vector<string> tokens, string inputFromClient);
        void showDownloads(vector<string> tokens, string inputFromClient);
        void logout(vector<string> tokens, string inputFromClient);
        void stopShare(vector<string> tokens, string inputFromClient);

        /**
         * @brief Downloads a file in a separate thread.
         * @param fileName The name of the file to download.
         * @param groupName The name of the group.
         * @param destinationPath The path to save the downloaded file.
         * @param fileSize The size of the file.
         * @param SHAs The SHA hashes of the file pieces.
         * @param pieceToSeeders Mapping from piece index to seeders.
         */
        void downloadFileThread(string fileName, string groupName, string destinationPath, int fileSize, vector<string> SHAs, unordered_map<int, vector<string>> pieceToSeeders);

        Leecher() = default; ///< Default constructor (private).
        ~Leecher() = default; ///< Default destructor (private).
        Leecher(const Leecher&) = delete; ///< Delete copy constructor.
        Leecher& operator=(const Leecher&) = delete; ///< Delete copy assignment operator.

        /**
         * @brief Parameterized constructor.
         * @param seederIp IP address of the seeder.
         * @param seederPort Port number of the seeder.
         */
        Leecher(string seederIp, int seederPort)
            : m_seederIp(seederIp)
            , m_seederPort(seederPort)
            , m_logger(Logger(seederIp, seederPort, "leecher"))
        {}

    public:
        /**
         * @brief Initializes the Leecher instance.
         */
        void init();

        /**
         * @brief Connects to the tracker server.
         * @param trackerIp IP address of the tracker server.
         * @param trackerPort Port number of the tracker server.
         */
        void connectTracker(string trackerIp, int trackerPort);

        /**
         * @brief Starts the Leecher instance.
         */
        void start();

        /**
         * @brief Stops the Leecher instance.
         */
        void stop();

        /**
         * @brief Retrieves the Singleton instance of the Leecher class.
         * @param seederIp IP address of the seeder.
         * @param seederPort Port number of the seeder.
         * @return Reference to the Singleton Leecher instance.
         */
        static Leecher& getInstance(string seederIp, int seederPort) {
            static Leecher m_instance(seederIp, seederPort);
            return m_instance;
        }
};


/**
 * @class Seeder
 * @brief Handles seeding operations for file-sharing.
 * 
 * The Seeder class manages incoming connections from leechers, processes commands 
 * related to file pieces, and sends appropriate responses. It ensures thread-safe 
 * access to shared resources and logs operations for debugging and tracking.
 */
class Seeder {
    private:
        string m_seederIp; ///< IP address of the seeder
        int m_seederPort; ///< Port number for the seeder
        ServerSocket m_seederSocket; ///< Socket used for communication
        Logger m_logger; ///< Logger for recording events

        /**
         * @brief Accepts incoming client connections and spawns handler threads.
         */
        void acceptConnections();

        /**
         * @brief Handles communication with a single leecher.
         * @param leecherSocketFd File descriptor of the leecher socket.
         */
        void handleLeecher(int leecherSocketFd);

        /**
         * @brief Executes a command received from a leecher.
         * @param command The command to be executed.
         * @param leecherSocketFd File descriptor of the leecher socket.
         * @return Response based on the executed command.
         * @throw string If command is invalid or execution fails.
         */
        string executeCommand(string command, int leecherSocketFd);

        Seeder() = default; ///< Default constructor is private to prevent instantiation.
        ~Seeder() = default; ///< Default destructor.
        Seeder(const Seeder&) = delete; ///< Delete copy constructor to prevent copying.
        Seeder& operator=(const Seeder&) = delete; ///< Delete assignment operator to prevent assignment.

        /**
         * @brief Parameterized constructor to initialize the seeder.
         * @param seederIp IP address of the seeder.
         * @param seederPort Port number for the seeder.
         */
        Seeder(string seederIp, int seederPort)
            : m_seederIp(seederIp)
            , m_seederPort(seederPort)
            , m_seederSocket(ServerSocket(seederIp, seederPort))
            , m_logger(Logger(seederIp, seederPort, "seeder"))
        {}

    public:
        /**
         * @brief Initializes the seeder by setting up the socket.
         */
        void init();

        /**
         * @brief Starts the seeder to accept connections.
         */
        void start();

        /**
         * @brief Stops the seeder by closing the socket.
         */
        void stop();

        /**
         * @brief Provides access to the singleton instance of Seeder.
         * @param seederIp IP address of the seeder.
         * @param seederPort Port number for the seeder.
         * @return Reference to the singleton instance of Seeder.
         */
        static Seeder& getInstance(string seederIp, int seederPort) {
            static Seeder m_instance(seederIp, seederPort);
            return m_instance;
        }
};

extern Logger generalLogger;
