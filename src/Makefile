CC=g++
CFLAGS=-Wall -O2

ODIR=obj

$(ODIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

_OBJ_INDEX = index.o
OBJ_INDEX = $(patsubst %,$(ODIR)/%,$(_OBJ_INDEX))

_OBJ_SEARCH = search.o
OBJ_SEARCH = $(patsubst %,$(ODIR)/%,$(_OBJ_SEARCH))

all: index search

index: $(OBJ_INDEX)
	$(CC) -o $@ $^ $(CFLAGS)

search: $(OBJ_SEARCH)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(ODIR)/*.o *~
