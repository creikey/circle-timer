.PHONY: all clean

OFILES=main.o

all: $(OFILES)
	gcc $(OFILES) `pkg-config --libs allegro-5 allegro_font-5 allegro_ttf-5 allegro_primitives-5`

main.o: main.c
	gcc -c main.c

clean:
	-rm *.o
	-rm *.gch
	-rm a.out
