CC = g++

GLLIBS = -lglut -lGLEW -lGL

all: primitives.cpp rectangle.cpp rectangle2.cpp at1.cpp
	
	$(CC) primitives.cpp ../lib/utils.cpp -o primitives $(GLLIBS)
	$(CC) at1.cpp ../lib/utils.cpp -o at1 $(GLLIBS)
	$(CC) rectangle.cpp ../lib/utils.cpp -o rectangle $(GLLIBS)
	$(CC) rectangle2.cpp ../lib/utils.cpp -o rectangle2 $(GLLIBS)

clean:
	rm -f triangle primitives rectangle rectangle2
