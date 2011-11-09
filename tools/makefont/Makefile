SDL_CFLAGS = `sdl-config --cflags`
SDL_LDFLAGS = `sdl-config --libs` -LSDL_gfx -lSDL_gfx 

all:
	gcc -o tiles2hdr -Wall tiles2hdr.c  $(SDL_CFLAGS) $(SDL_LDFLAGS)
	./tiles2hdr 4x8font.bmp && cp tiles.h ../include && cp tiles.c ../src

clean:
	rm -f tiles.c tiles.h tiles2hdr

