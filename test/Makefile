CXX := c++
CXXFLAGS := -std=c++11 -O2 -Wall -Wextra -pedantic -g
INCLUDE := -I../include -I../rapidjson/include -I../catch/single_include
TARGET := autojsoncxx_catch

PYTHON3 := python3
DEFINITION_FILE := ../examples/userdef.json
GENERATED_HPP := userdef.hpp

all: $(TARGET)

$(TARGET): main.o test.o $(GENERATED_HPP)
	$(CXX) $(CXXFLAGS) main.o test.o -o $(TARGET)
	
$(GENERATED_HPP): $(DEFINITION_FILE)
	$(PYTHON3) ../autojsoncxx.py --out=$(GENERATED_HPP) $(DEFINITION_FILE)

main.o: main.cpp $(GENERATED_HPP)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c main.cpp -o main.o

test.o: test.cpp $(GENERATED_HPP)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c test.cpp -o test.o
	
.PHONY: clean test

test:
	./$(TARGET) --success

clean:
	rm *.o $(TARGET) $(GENERATED_HPP)
