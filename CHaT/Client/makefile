CXX = g++
CXXFLAGS = -Wall -std=c++17 -pthread
LDLIBS = -lncurses
TARGET = client
SRC = main.cpp Client.cpp

.PHONY: all clean run test

all: $(TARGET)

$(TARGET):
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET)
