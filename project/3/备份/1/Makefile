CFLAGS = -Wall -Werror -pedantic -ggdb3 -std=gnu++98
all: player ringmaster
player:player.cpp potato.hpp
	g++ $(CFLAGS) -o player player.cpp potato.hpp
ringmaster:ringmaster.cpp potato.hpp
	g++ $(CFLAGS) -o ringmaster ringmaster.cpp potato.hpp
# all: test
# test:test.cpp potato.hpp
# 	g++ $(CFLAGS) -o test test.cpp potato.hpp
