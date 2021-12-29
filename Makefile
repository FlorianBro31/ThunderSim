COMPILER:= gcc

all:
	$(COMPILER) ThunderSim.c -lSDL2 -o thundersim