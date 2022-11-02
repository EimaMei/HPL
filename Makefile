CC = clang++
OUTPUT = build
EXE = build/hcl
PLATFORM = nothing

SRC = $(basename $(wildcard source/*.cpp))
OBJ = $(addprefix $(OUTPUT)/,$(addsuffix .o,$(notdir $(SRC))))

ifeq ($(OS),Windows_NT)
    PLATFORM = windows
else
    PLATFORM := $(shell uname -s)
endif

FLAGS = -std=c++17 -O2 -Wall -Wpedantic
LIBS = -L"source/deps/$(PLATFORM)" -lSOIL2
INCLUDE = -I"include"

all: $(EXE) run
%.hpp: $(EXE)

$(OUTPUT):
	@mkdir $@

$(OUTPUT)/%.o: source/%.cpp
	$(CC) $(FLAGS) $(INCLUDE) $^ -c -o $@

$(EXE): $(OBJ) main.cpp
	$(CC) $(FLAGS) $(INCLUDE) $(OBJ) main.cpp $(LIBS) -o $@

run: $(EXE)
	./$(EXE) examples/general/main.hcl

clean:
	rm -rf build/**

debug: $(EXE)
	lldb ./$(EXE)