CXX       := g++
CXX_FLAGS := -std=c++11 -ggdb

SRC     := src
INCLUDE := inc

LIBRARIES   :=
EXECUTABLE  := linker


all: $(EXECUTABLE)

run: clean all
	clear
	$(EXECUTABLE)

$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) $^ -o $@ $(LIBRARIES)

clean:
	-rm $(EXECUTABLE)
