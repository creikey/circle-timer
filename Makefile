exedir=/usr/local/bin
installdir=/opt/circle-timer
exename=circle-timer

.PHONY: all clean install uninstall debug

OFILES=main.o

all: $(OFILES)
	gcc $(OFILES) -o $(exename) `pkg-config --libs --static allegro-static-5 allegro_font-static-5 allegro_ttf-static-5 allegro_primitives-static-5`

install: all
	-sudo mkdir -p $(installdir)
	sudo cp $(exename) $(installdir)
	sudo cp -r ttf/ $(installdir)/
	sudo ln -s $(installdir)/$(exename) $(exedir)/$(exename)

uninstall:
	-sudo rm -r $(installdir)
	-sudo rm $(exedir)/$(exename)

debug:
	gcc -c $(OFILES) -g
	gcc $(OFILES) -g `pkg-config --libs allegro-5 allegro_font-5 allegro_ttf-5 allegro_primitives-5`

main.o: main.c
	gcc -c main.c

clean:
	-rm *.o
	-rm *.gch
	-rm a.out
