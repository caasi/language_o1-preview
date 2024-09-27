# Compiler
CC = gcc

# Compiler Flags
CFLAGS = -Wall -Wextra -std=c11 -g

# Include Directories
INCULDES = -I.

# Source Files
SRCS = main.c lexer.c parser.c env.c symbol_table.c evaluator.c print.c

# Object Files
OBJS = $(SRCS:.c=.o)

# Dependency Files
DEPS = $(OBJS:.o=.d)

# Executable Name
TARGET = lang

# Default Target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	$(CC) -MM $(CFLAGS) $(INCLUDES) $< > $(@:.o=.d)

# Include dependency files
-include $(DEPS)

test: all
	./run_tests.sh

# Clean up generated files
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

# Phony Targets
.PHONY: all test clean
