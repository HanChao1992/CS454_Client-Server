all: stringServer stringClient

FLAGS = -DNDEBUG -ggdb -pedantic -Wextra -Wall

stringServer: stringServer.cpp
	g++ $(FLAGS) stringServer.cpp -o stringServer

stringClient: stringClient.cpp
	g++ $(FLAGS) -pthread stringClient.cpp -o stringClient

clean:
	rm -f *.o stringServer stringClient
