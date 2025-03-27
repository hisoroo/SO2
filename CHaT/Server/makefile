CXX = g++
CXXFLAGS = -Wall -std=c++11 -pthread

TARGET = server
SRC = main.cpp Server.cpp

.PHONY: all clean run test

all: $(TARGET)

$(TARGET):
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET)

