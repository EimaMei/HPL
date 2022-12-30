CC = clang++
OUTPUT = build
EXE = build/hpl
PLATFORM = nothing

SRC = $(basename $(wildcard source/*.cpp))
OBJ = $(addprefix $(OUTPUT)/,$(addsuffix .o,$(notdir $(SRC))))

FLAGS = -std=c++17 -O2 -Wall -Wpedantic
LIBS = -L"source/deps/$(PLATFORM)" -lSOIL2
INCLUDE = -I"include"
HPL-INPUT = examples/general/main.hpl

ifeq ($(OS),Windows_NT)
    PLATFORM = windows
	EXE := $(EXE).exe
	FLAGS = -std=c++20 -O2 -Wall -Wpedantic
# For whatever reason, Windows clang requires C++20
else
    PLATFORM := $(shell uname -s | tr [:upper:] [:lower:])
endif


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

runLogs: $(EXE)
	./$(EXE) -log $(HPL-INPUT)

runDebug: $(EXE)
	./$(EXE) -g $(HPL-INPUT)

runJson: $(EXE)
	./$(EXE) -dumpJson $(HPL-INPUT)

clean:
	rm -rf build/**

debug: $(EXE)
	lldb ./$(EXE)