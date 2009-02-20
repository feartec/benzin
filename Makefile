GENERICOBJECTS = main.o general.o memfile.o xml.o endian.o
MAINOBJECTS = brlyt.o brlan.o
OBJECTS = $(GENERICOBJECTS) $(MAINOBJECTS)
LIBS = -lmxml
OUTPUT = benzin
main: $(OBJECTS)
	gcc -o $(OUTPUT) $(LIBS) $(OBJECTS)
clean:
	rm -f $(OUTPUT) $(OBJECTS)
