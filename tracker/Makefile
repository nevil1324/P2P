CC = g++
CFLAGS = -Wall -I/usr/include/openssl
LDFLAGS = -lssl -lcrypto
DEPS = headers.h  
OBJ = classes/Logger.o classes/Groups.o classes/ServerSocket.o classes/Users.o classes/Utils.o classes/Tracker.o tracker.o

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

tracker: $(OBJ)
	$(CC) $(CFLAGS) -pthread -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *.o tracker ./classes/*.o