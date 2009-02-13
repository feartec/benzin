OBJECTS = main.o general.o brlyt.o brlan.o
OUTPUT = benzin
main: $(OBJECTS)
	gcc -o $(OUTPUT) $(OBJECTS)
clean:
	rm -f $(OUTPUT) $(OBJECTS)
