OBJECTS = main.o general.o brlyt.o brlan.o memfile.o xml.o
LIBS = -lmxml
OUTPUT = benzin
main: $(OBJECTS)
	gcc -o $(OUTPUT) $(LIBS) $(OBJECTS)
clean:
	rm -f $(OUTPUT) $(OBJECTS)
