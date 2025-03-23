CXX = g++
CXXFLAGS = -std=c++14 -Wall -O2 -pthread

TARGET = dpp
SRC = dpp.cpp

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

