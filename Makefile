CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -Iinclude
SRCDIR = src
BINDIR = bin
TARGET = $(BINDIR)/sicasm

SRCS = \
    $(SRCDIR)/main.c \
    $(SRCDIR)/p1_read_source.c \
    $(SRCDIR)/p1_assign_loc.c \
    $(SRCDIR)/p1_assign_sym.c \
    $(SRCDIR)/access_int_file.c \
    $(SRCDIR)/access_symtab.c \
    $(SRCDIR)/p2_search_optab.c \
    $(SRCDIR)/p2_assemble_inst.c \
    $(SRCDIR)/p2_write_obj.c \
    $(SRCDIR)/p2_write_list.c
OBJS = $(SRCS:.c=.o)

all: dirs $(TARGET)

dirs:
	mkdir -p $(BINDIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(SRCDIR)/*.o $(TARGET)

.PHONY: all clean dirs
