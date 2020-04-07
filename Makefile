.PHONY: clean run #Tell make that not associated with building files

#Compile source files and produce executable
all : warn.o
	cc -o warn warn.c
#Remove object and temp files
clean :
	rm warn *.o
#Runs compiled executable
run :
	./warn
