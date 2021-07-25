# shell might be inherited from the environment
SHELL = /bin/sh

# define suffix list
.SUFFIXES:
.SUFFIXES: .c .o .h

INC_DIR = include
SRC_DIR = samples
OBJ_DIR = build

CC = gcc
CFLAGS = -I.

SRCS = $(SRC_DIR)/play.c
OBJS = $(OBJ_DIR)/play

SRC2 = ${SRC_DIR}/cli.c
OBJ2 = ${OBJ_DIR}/cli

DEPS = ${INC_DIR}/machyapi.h

samples: dir ${OBJS} ${OBJ2}

dir:
	mkdir -p build

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $< -o $@

$(OBJS): $(DEPS)

${OBJ2}: ${SRC2}
	${CC} $(CFLAGS) $< -o $@

${OBJ2}: ${DEPS}