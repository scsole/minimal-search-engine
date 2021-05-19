CC=g++
CFLAGS=-std=c++11 -Wall -O2

ODIR=obj

$(ODIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

_OBJ_PARSE = parse.o
OBJ_PARSE = $(patsubst %,$(ODIR)/%,$(_OBJ_PARSE))

_OBJ_INDEX = index.o
OBJ_INDEX = $(patsubst %,$(ODIR)/%,$(_OBJ_INDEX))

_OBJ_SEARCH = search.o
OBJ_SEARCH = $(patsubst %,$(ODIR)/%,$(_OBJ_SEARCH))

all: parse index search

parse: $(OBJ_PARSE)
	$(CC) -o $@ $^ $(CFLAGS)

index: $(OBJ_INDEX)
	$(CC) -o $@ $^ $(CFLAGS)

search: $(OBJ_SEARCH)
	$(CC) -o $@ $^ $(CFLAGS)

clean-all: clean clean-index

clean:
	rm -f $(ODIR)/*.o *~

clean-index:
	rm -f *.bin