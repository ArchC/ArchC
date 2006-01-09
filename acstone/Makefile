
CC = gcc
CFLAGS = -g

SUFFIX = .x86

TESTS = $(patsubst %.c,%$(SUFFIX),$(wildcard *.c))

# Use rules
help:
	@echo -e "\nRules:\n"
	@echo -e "help: Show this help"
	@echo -e "build: Compile programs"
	@echo -e "clean: Remove generated files"
	@echo -e "all: clean build\n"


# Compile programs
build: $(TESTS)

$(TESTS): %$(SUFFIX): %.c
	$(CC) $(CFLAGS) $< -o $@


# Clean executables and backup files
clean: 
	$(foreach test,$(TESTS),rm -f $(test))
	rm -f *~
	rm -f *.cmd
	rm -f *.out

# Clean executables, backup files and compile programs
all: clean build


.PHONY: build clean all
