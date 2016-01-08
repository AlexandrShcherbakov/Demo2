CFLAGS += '-std=c99'
CFLAGS += -I'external/include'
CFLAGS += '-Wall'
LDFLAGS += -L'external/lib/' 
LDFLAGS += -L'external/lib/sdl/'
LDLIBS += -lSDL2 -lSDL2main -lm -lGL -lclew

main: main.o types.o glElements.o clElements.o