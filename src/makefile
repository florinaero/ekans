
IDIR = include/
SRCDIR = src/
DEPS := $(wildcard $(IDIR)*.hpp) #find hpp files
CSRC := $(wildcard $(SRCDIR)*.cpp) #find all cpp files
LIBS := -lncurses

CC=g++
CFLAGS=-I $(IDIR) -std=c++17 -g -W
EXSRC = $(shell if [ ! -d "$(SRCDIR)" ]; then echo "Must create src directory!" ; fi)
EXOBJ = $(shell if [ ! -d "$(ODIR)" ]; then mkdir $(ODIR) && echo "obj dir created" ; fi)
EXINCL = $(shell if [ ! -d "$(IDIR)" ]; then echo "Must create include directory!" ; fi)

#object file folder
ODIR = obj/
OBJ := $(CSRC:.cpp=.o) #replace .cpp with .o
OBJ := $(OBJ:$(SRCDIR)%=$(ODIR)%) #replace /src with /obj  
CHMOD = $(shell chmod 755 $(OBJ))	

$(ODIR)%.o: $(SRCDIR)%.cpp $(DEPS) 	#Define rule that creates object files
# Compile cpp files and names as the name from left of :
	$(CC) -c -o $@ $< $(CFLAGS)
	
result.o: rule $(OBJ)
	$(CC) -o  $@ $(OBJ) $(CFLAGS) $(LIBS)
	$(CHMOD)
	@echo "Success!"

rule:
	@echo $(EXOBJ)
	@echo $(EXSRC)
	@echo $(EXINCL)

# make "clean" to be an out-of-date target for make, in case of a "clean" file
#	in your main directory it will not be considered by make
.PHONY: clean
clean:
		rm -f $(ODIR)*.o
