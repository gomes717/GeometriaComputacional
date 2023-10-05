CC = g++

GLLIBS = -lglut -lGLEW -lGL -lm

all: ./src/main.cpp ./src/dcel.cpp 
	
	$(CC) ./src/main.cpp ./src/dcel.cpp ./lib/utils.cpp -o dcel $(GLLIBS)

clean:
	rm -f triangle primitives rectangle rectangle2

run: all
	./dcel