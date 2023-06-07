all:
	g++ -std=c++11 -framework CoreGraphics -framework ApplicationServices -o OneClickQuits source/hashset.cpp main.cpp