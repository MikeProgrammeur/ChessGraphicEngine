# ===========================================================================
#  Chess — cross-platform Makefile (macOS / Linux / Windows)
# ===========================================================================

# --- OS detection ---
UNAME_S := $(shell uname -s)

ifeq ($(OS),Windows_NT)
    OS_GROUP := windows
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        OS_GROUP := macos
    else
        OS_GROUP := linux
    endif
endif

# --- Source files ---
SOURCES = main.cpp textures.cpp layout.cpp rendering.cpp engine.cpp
OBJECTS = $(SOURCES:.cpp=.o)

CXX = g++

# --- Common flags ---
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# ===========================================================================
#  Windows (MinGW-w64)
# ===========================================================================
ifeq ($(OS_GROUP),windows)
    SFML_DIR ?= C:/SFML-3.1.0
    TARGET = main.exe
    CXXFLAGS += -I$(SFML_DIR)/include
    LDFLAGS = -L$(SFML_DIR)/lib -static-libgcc -static-libstdc++
    LDLIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
    LDLIBS += -lgdi32 -lwinmm
    RM = del /Q
    MKDIR_P = if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
endif

# ===========================================================================
#  macOS
# ===========================================================================
ifeq ($(OS_GROUP),macos)
    SFML_DIR ?= /Users/Username/Downloads/SFML-3.1.0
    TARGET = main
    CXXFLAGS += -I$(SFML_DIR)/include
    LDFLAGS = -L$(SFML_DIR)/lib -Wl,-rpath,$(SFML_DIR)/lib
    LDLIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
    RM = rm -f
endif

# ===========================================================================
#  Linux
# ===========================================================================
ifeq ($(OS_GROUP),linux)
    SFML_DIR ?= /usr/local
    TARGET = main
    CXXFLAGS += -I$(SFML_DIR)/include
    LDFLAGS = -L$(SFML_DIR)/lib -Wl,-rpath,$(SFML_DIR)/lib
    LDLIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
    RM = rm -f
endif

# --- Rules ---
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean fclean re

clean:
	$(RM) $(OBJECTS)

fclean: clean
	$(RM) $(TARGET)

re: fclean all
