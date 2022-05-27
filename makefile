all: fish.c pellet.c swim_mill.c
	gcc -o swim_mill swim_mill.c -pthread
	gcc -o fish fish.c -pthread
	gcc -o pellet pellet.c -pthread