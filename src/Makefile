CC=g++
CFLAGS=-Wall -O2

ODIR=obj

_OBJ = index.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

index: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(ODIR)/*.o *~
