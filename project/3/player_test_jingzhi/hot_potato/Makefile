TARGETS=player ringmaster 
CFLAGS=-Wall -Werror -pedantic -std=gnu++98

all: $(TARGETS)

socketutils.o: socketutils.cpp
	g++ $(CFLAGS) -c socketutils.cpp

player: player.o socketutils.o
	g++ -o player player.o socketutils.o

player.o: player.cpp potato.hpp
	g++ $(CFLAGS) -c player.cpp

ringmaster: ringmaster.o socketutils.o
	g++ -o ringmaster ringmaster.o socketutils.o

ringmaster.o: ringmaster.cpp potato.hpp
	g++ $(CFLAGS) -c ringmaster.cpp

clean:
	rm -f *~ *.o $(TARGETS)