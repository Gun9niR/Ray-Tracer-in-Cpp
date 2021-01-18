CXX = g++
CXX_FLAGS = -std=c++11
EXE = raytracer
SRC_DIR = src
DEPS = $(shell find . -name "*.h")

$(EXE): $(SRC_DIR)/main.cpp $(DEPS)
	$(CXX) -o $@ $(SRC_DIR)/main.cpp $(CXX_FLAGS)

.PHONY: clean

clean:
	rm $(EXE)