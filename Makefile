# --- Variables ---
# On utilise CXX pour le C++ (CC est traditionnellement réservé au C)
CXX = g++

# Options de compilation (On active les avertissements de base)
CXXFLAGS = -Wall -Wextra -O2

# Bibliothèques à lier (L'ordre est très important pour SFML !)
LDLIBS = -lsfml-graphics -lsfml-window -lsfml-system

SOURCES = main.cpp
# On remplace .cpp par .o (et non .c par .o)
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = main.exe

# --- Règles ---

# Règle par défaut
all: $(TARGET)

# 1. Lien de l'exécutable (Assemblage final)
# $@ -> $(TARGET)
# $^ -> $(OBJECTS)
$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDLIBS)

# 2. Compilation des fichiers .cpp en .o
# $< -> Premier prérequis (le fichier .cpp)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Déclaration des cibles phony
.PHONY: all clean fclean re

# Nettoyage des fichiers objets
clean:
	rm -f $(OBJECTS)

# Nettoyage complet (fichiers objets et exécutable)
fclean: clean
	rm -f $(TARGET) $(TARGET).exe

# Refaire la compilation
re: fclean all