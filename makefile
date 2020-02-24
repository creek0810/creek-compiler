SRC_DIR = src/
LIB_DIR = src/lib

SRC = $(wildcard $(SRC_DIR)*.c)
OBJS = $(SRC:%.c=%.o)

compiler: $(OBJS)
	cc -o compiler $(OBJS)

.PHONY: clean
clean:
	rm -f $(SRC_DIR)*.o