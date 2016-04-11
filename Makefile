CXX = g++

CFLAGS = -Ofast -fdata-sections -ffunction-sections -fomit-frame-pointer -I. -Wunused-result
LDFLAGS = -lSDL -Wl,--gc-sections -Wl,--as-needed -flto -lm
OUTPUT = yapeSDL

SOURCES = \
archdep.cpp		\
cpu.cpp	\
diskfs.cpp \
dos.cpp \
drive.cpp \
iec.cpp \
interface.cpp		\
keyboard.cpp \
main.cpp \
prg.cpp \
serial.cpp	\
tape.cpp \
tcbm.cpp \
tedmem.cpp	
#bam.cpp			\
#diskimg.cpp \
OBJS = ${SOURCES:.cpp=.o}

all: ${OUTPUT}

${OUTPUT}:${OBJS}
	${CXX} -o ${OUTPUT} ${SOURCES} ${CFLAGS} ${LDFLAGS} ${DEFINES} 
	
clean:
	rm *.o ${OUTPUT}
