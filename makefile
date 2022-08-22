CC = gcc
CFLAGS = -Wall -ansi -pedantic
LDFLAGS = -lm
OBJFILES = main.o preprocessor.o utilities.o assembly.o data.o command.o output.o
TARGET = assembler

all: $(TARGET)
	
$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES)

clean:
	rm -f $(OBJFILES) $(TARGET) *~