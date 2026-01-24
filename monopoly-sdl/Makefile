# Makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./src
LDFLAGS = -lSDL2

TARGET = monopoly_engine
SRC = src/main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
