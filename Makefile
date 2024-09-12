CC=g++
CXXFLAGS=-Wall -Ijson/include

client: client.cpp requests.cpp helpers.cpp buffer.cpp
	$(CC) $(CXXFLAGS) -o client client.cpp requests.cpp helpers.cpp buffer.cpp -Wall

run: client
	./client

clean:
	rm -f *.o client
