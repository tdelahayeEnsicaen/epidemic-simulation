CC=gcc
CFLAGS=-Wall -Wextra -pedantic -DMUTEX
CPPFLAGS=-Iinclude

all: timer citizen_manager epidemic_sim

obj/%.o: src/%.c include/%.h
	mkdir -p obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

citizen_manager: obj/CitizenManager.o obj/Map.o
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -lrt -lpthread -o $@ 

epidemic_sim: obj/Map.o obj/EpidemicSim.o
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -lrt -lpthread -o $@ 

obj/timer.o: src/timer.c 
	mkdir -p obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

timer: obj/timer.o
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ 

clean:
	rm obj/*.o

distclean:
	rm timer citizen_manager epidemic_sim