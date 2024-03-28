# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lgmp

# Source file
SRC = main.c

# Output file
OUTPUT = output.txt

# Temporary files
TEMPS = temp_factorial.txt temp_process_list.txt temp_average.txt

# Target to build the program
program: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o program $(LDFLAGS)
# Target to run the program
run: program
	@./program input.txt $(OUTPUT)

# Target to clean up files
clean:
	@echo "Cleaning up..."
	@truncate -s 0 $(OUTPUT) 2>/dev/null || true
	@rm -f $(TEMPS) 2>/dev/null || true

cleanall: clean
	@rm -f program 2>/dev/null || true


# Phony targets
.PHONY: clean cleanall run program