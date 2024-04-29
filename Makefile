all: main

main: main.c src/emulator.c 
	gcc -o main main.c src/emulator.c -I src/include -L src/lib -l mingw32 -l SDL2main -l SDL2

run: main
	./main

	
