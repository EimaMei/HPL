CC = clang++
OUTPUT = build
EXE = build/hpl
PLATFORM = nothing

SRC = $(basename $(wildcard source/*.cpp))
OBJ = $(addprefix $(OUTPUT)/,$(addsuffix .o,$(notdir $(SRC))))

ifeq ($(OS),Windows_NT)
    PLATFORM = windows
else
    PLATFORM := $(shell uname -s | tr [:upper:] [:lower:])
endif

FLAGS = -std=c++17 -O2 -Wall -Wpedantic
LIBS = -L"source/deps/$(PLATFORM)" -lSOIL2
INCLUDE = -I"include"
HPL-INPUT = examples/nation/main.hpl

all: $(OUTPUT) $(EXE) run
%.hpp: $(EXE)

$(OUTPUT):
	@mkdir $@

$(OUTPUT)/%.o: source/%.cpp
	$(CC) $(FLAGS) $(INCLUDE) $^ -c -o $@

$(EXE): $(OBJ) main.cpp
	$(CC) $(FLAGS) $(INCLUDE) $(OBJ) main.cpp $(LIBS) -o $@

run: $(EXE)
	./$(EXE) $(HPL-INPUT)

runDebug: $(EXE)
	./$(EXE) -l $(HPL-INPUT)

clean:
	rm -rf build/**

debug: $(EXE)
	lldb ./$(EXE)