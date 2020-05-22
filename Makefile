VPATH=./src/ ./src/emu ./src/core ./src/cache
OBJDIR=./obj/
EXEC=Monichi

CC=gcc
CPP=g++

COMMON=-Isrc/ -Isrc/emu -Isrc/core -Isrc/cache
CFLAGS=-O3
LDFLAGS=-lm

OBJ=sim.o Cache.o Core.o BranchPredictor.o FunctionUnit.o ReorderBuffer.o
OBJ+=ArchitectureState.o Emulator.o Etc.o Instruction.o Memory.o SystemManager.o

EXECOBJ=$(addprefix $(OBJDIR), $(OBJ))
DEPS=$(wildcard src/*) $(wildcard src/emu/*) $(wildcard src/core/*) $(wildcard src/cache/*) Makefile

all: obj $(EXEC)

$(EXEC): $(EXECOBJ)
	$(CPP) $(COMMON) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJDIR)%.o: %.cpp $(DEPS)
	$(CPP) $(COMMON) $(CFLAGS) -c $< -o $@

obj:
	mkdir -p obj

.PHONY: clean

clean:
	rm -rf $(EXEC) $(EXECOBJ) $(OBJDIR)/*
