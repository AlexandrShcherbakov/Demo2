CFLAGS += '-std=c99'
CFLAGS += -I'../sdl/include'
CFLAGS += -I'..'
LDFLAGS += -L'./../sdl/lib/x64' 
LDLIBS += -lSDL2 -lSDL2main -lm -lGL

main: main.o types.o glElements.o