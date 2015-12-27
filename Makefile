CFLAGS += '-std=c99'
CFLAGS += -I'../sdl/include'
CFLAGS += -I'..'
CFLAGS += -I'../Radiosity/CL'
CFLAGS += -I'../clew'
CFLAGS += '-Wall'
LDFLAGS += -L'./../sdl/lib/x64' 
LDFLAGS += -L'./../clew/bin/Release'
LDLIBS += -lSDL2 -lSDL2main -lm -lGL -lclew

main: main.o types.o glElements.o clElements.o