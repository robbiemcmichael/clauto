PROJECT = clauto
DEP     = dep.mk

# Change this as required
INCPATH = /usr/local/cuda/include/

CC      = gcc
CFLAGS  = -std=c99 -I$(INCPATH)
LINK    = -L. -lm -lclAppleFft -lOpenCL -lstdc++

SOURCES = cl_abstractions.c cl_error.c convert.c data_handling.c fft.c main.c \
              options.c spectrum.c sum.c
OBJECTS = $(SOURCES:.c=.o)

$(PROJECT) : $(DEP) $(OBJECTS) $(STATIC)
	$(CC) $(CFLAGS) -o $(PROJECT) $(OBJECTS) $(LINK) $(STATIC)

$(DEP) : $(SOURCES)
	$(CC) -MM -x c $(SOURCES) > $(DEP)

%.o : %.c
	$(CC) $(CFLAGS) -c $<
-include $(DEP)

clean :
	rm -f $(DEP)
	rm -f $(OBJECTS)
