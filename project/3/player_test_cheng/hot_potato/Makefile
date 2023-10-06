CFLAGS=-std=gnu++98 -ggdb3 
PROGS=player ringmaster

all: $(PROGS)
clean:
	rm -f $(PROGS)

player: player.cpp
	g++ $(CFLAGS) -o $@ $<

ringmaster: ringmaster.cpp
	g++ $(CFLAGS) -o $@ $<

