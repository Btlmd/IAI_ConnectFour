.PHONY: main

all: main



main:
	g++ -O2 --std=c++17 -o main \
		sdk/main.cpp sdk/json/jsoncpp.cpp Judge.cpp Strategy.cpp \
		operate/Decision.hpp operate/Game.cpp operate/Node.cpp
