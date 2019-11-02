all: buildrooms adventure

buildrooms: shandst.buildrooms.c
	gcc -Wall -g shandst.buildrooms.c -o buildrooms

adventure: shandst.adventure.c
	gcc -Wall -g -lpthread shandst.adventure.c -o adventure

clean:
	rm buildrooms adventure 
