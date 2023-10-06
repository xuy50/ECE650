TARGETS=ringmaster player

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

ringmaster: ringmaster.cpp hot_potato.cpp
	g++ -g -o ringmaster ringmaster.cpp hot_potato.cpp

player: player.cpp hot_potato.cpp
	g++ -g -o player player.cpp hot_potato.cpp

