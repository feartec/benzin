OBJECTS=main.o
main: $(OBJECTS)
	gcc -o main $(OBJECTS)
clean:
	rm -f main $(OBJECTS)
