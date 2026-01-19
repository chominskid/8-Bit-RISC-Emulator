TARGET := emulator

CXX := g++
CXXFLAGS := -Wall -Wextra -O3 -std=c++20

SRCS_COMMON := $(shell find src/emulator src/utils -name "*.cpp")
OBJS_COMMON := $(SRCS_COMMON:.cpp=.o)

SRCS_FRONTEND := $(shell find src/frontend -name "*.cpp")
OBJS_FRONTEND := $(SRCS_FRONTEND:.cpp=.o)
LIBS_FRONTEND := -lsfml-system -lsfml-window -lsfml-graphics

SRCS_FRONTEND_HEADLESS := $(shell find src/frontend_headless -name "*.cpp")
OBJS_FRONTEND_HEADLESS := $(SRCS_FRONTEND_HEADLESS:.cpp=.o)

all: emulator emulator_headless

emulator: $(OBJS_COMMON) $(SRCS_FRONTEND)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS_FRONTEND)

emulator_headless: $(OBJS_COMMON) $(SRCS_FRONTEND_HEADLESS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS_COMMON) $(OBJS_FRONTEND) $(OBJS_FRONTEND_HEADLESS) $(TARGET)

.PHONY: all clean