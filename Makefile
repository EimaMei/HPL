CC = clang++
OUTPUT = build
EXE = build/hcl

SRC = $(basename $(wildcard source/*.cpp))
OBJ = $(addprefix $(OUTPUT)/,$(addsuffix .o,$(notdir $(SRC))))

FLAGS = -std=c++17 -O2 -Wpedantic
LIBS = -L"$(OUTPUT)"
INCLUDE = -I"include"

$(OUTPUT)/%.o: source/%.cpp
	$(CC) $(FLAGS) $(INCLUDE) $^ -c -o $@

all: $(EXE) run
%.hpp: $(EXE)

$(EXE): $(OBJ) main.cpp
	$(CC) $(FLAGS) $(INCLUDE) $(OBJ) main.cpp $(LIBS) -o $@

run: $(EXE)
	./$(EXE) -g examples/event/main.hcl

clean:
	rm -rf build/**

debug: $(EXE)
	lldb ./$(EXE)