CC = g++
CFLAGS = -Wall -I/usr/include/openssl -Wno-deprecated-declarations
LDFLAGS = -lssl -lcrypto
DEPS = headers.h  
OBJ = classes/Logger.o classes/ThreadPool.o classes/ClientSocket.o classes/ServerSocket.o classes/Files.o classes/Leecher.o classes/Seeder.o classes/Utils.o client.o

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

client: $(OBJ)
	$(CC) $(CFLAGS) -pthread -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *.o client ./classes/*.o