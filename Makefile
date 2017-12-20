C_FILES=mhash.c mhtbl.c mhtbl.h obvdb.c obvdb.h utils.c
SRCS=$(addprefix src/,$(C_FILES))

all:
	gcc -g $(SRCS) -o mhash -pthread

clean:
	rm -rf mhash
