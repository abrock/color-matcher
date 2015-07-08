default:
	g++ -std=c++11 -Wall -pedantic -O0 -g -ggdb `pkg-config opencv --cflags` `pkg-config opencv --libs`  -o color-matcher main.cpp

tex:
	latexmk -pdf main
