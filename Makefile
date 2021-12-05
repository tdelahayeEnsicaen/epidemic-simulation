CC=gcc
CFLAGS=-Wall -Wextra -pedantic -DMUTEX -g
CPPFLAGS=-Iinclude

all: timer citizen_manager epidemic_sim press_agency

clean:
	rm obj/*.o

distclean:
	rm timer citizen_manager epidemic_sim press_agency

# ------------------------- COMMON ------------------------

obj/Map.o: src/Map.c include/Map.h include/Utils.h include/Doctor.h
	mkdir -p obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

obj/Process.o: src/Process.c include/Process.h
	mkdir -p obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

obj/Utils.o: src/Utils.c include/Utils.h
	mkdir -p obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

# -------------------- CITIZEN MANAGER --------------------

obj/Citizen.o: src/Citizen.c include/Citizen.h include/Map.h include/Utils.h
	mkdir -p obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

obj/Doctor.o: src/Doctor.c include/Doctor.h include/Map.h
	mkdir -p obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

obj/Firefigther.o: src/Firefigther.c include/Firefigther.h include/Map.h include/Utils.h
	mkdir -p obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

obj/Journalist.o: src/Journalist.c include/Journalist.h include/Map.h
	mkdir -p obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

obj/CitizenManager.o: src/CitizenManager.c include/CitizenManager.h include/Process.h include/Map.h include/Citizen.h include/Doctor.h include/Firefigther.h include/Journalist.h
	mkdir -p obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

citizen_manager: obj/CitizenManager.o obj/Process.o obj/Utils.o obj/Map.o obj/Citizen.o obj/Doctor.o obj/Firefigther.o obj/Journalist.o
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -lrt -lpthread -o $@ 

# ------------------ EPIDEMIC SIMULATION ------------------

obj/DataCollector.o: src/DataCollector.c include/DataCollector.h include/Map.h
	mkdir -p obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

obj/EpidemicSim.o: src/EpidemicSim.c include/EpidemicSim.h include/Process.h include/Map.h include/Utils.h include/DataCollector.h include/Firefigther.h include/Doctor.h
	mkdir -p obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

epidemic_sim: obj/EpidemicSim.o obj/Process.o obj/Map.o obj/Utils.o obj/DataCollector.o obj/Firefigther.o
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -lrt -lpthread -lncurses -o $@ 

# ------------------------- TIMER -------------------------

obj/timer.o: src/timer.c include/Process.h
	mkdir -p obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

timer: obj/timer.o obj/Process.o obj/Utils.o
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -lrt -lpthread -o $@ 

# ---------------------- PRESS AGENCY ---------------------

obj/PressAgency.o: src/PressAgency.c include/PressAgency.h include/Process.h include/Map.h
	mkdir -p obj
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

press_agency: obj/PressAgency.o obj/Process.o obj/Map.o obj/Utils.o
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -lrt -lpthread -o $@ 
