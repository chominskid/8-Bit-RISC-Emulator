TARGET := emulator

CXX := g++
CXXFLAGS := -Wall -Wextra -O3 -std=c++20
LIBS := -lsfml-system -lsfml-window -lsfml-graphics

SRCS := $(shell find . -name "*.cpp")
OBJS := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean