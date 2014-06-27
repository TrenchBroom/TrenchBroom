CXX=g++
CC=gcc
WXFLAGS=$(shell wx-config --cxxflags)
#CFLAGS=-Wall -g
CFLAGS=-Wall -O2 -fomit-frame-pointer -march=native
TARGET_OUTPUT_DIR=bin

INCLUDE=-I. -I../Source -I/usr/include/freetype2
SRC=$(wildcard *.cpp ../Source/*/*.cpp ../Source/*/*/*.cpp)
OBJ=$(SRC:.cpp=.o)

release: $(TARGET_OUTPUT_DIR)/TrenchBroom
	@strip $(TARGET_OUTPUT_DIR)/TrenchBroom

$(TARGET_OUTPUT_DIR)/TrenchBroom: $(OBJ) $(TARGET_OUTPUT_DIR)/Resources Version.h
	$(CXX) $(CFLAGS) $(OBJ) $(shell wx-config --libs) $(shell wx-config --gl-libs) -lGL -lGLEW -lfreetype -o $@

Version.h:
	@./IncBuildNo.sh

$(TARGET_OUTPUT_DIR)/Resources:
	mkdir -p $(TARGET_OUTPUT_DIR)/Resources
	cp -r ../Resources/Help/Documentation ../Resources/Defs $(TARGET_OUTPUT_DIR)/Resources/
	cp ../Resources/Fonts/* ../Resources/Graphics/*.png ../Resources/Graphics/QuakePalette.lmp $(TARGET_OUTPUT_DIR)/Resources/
	cp ../Source/Renderer/Shader/*.vertsh ../Source/Renderer/Shader/*.fragsh $(TARGET_OUTPUT_DIR)/Resources/

%.o: %.cpp
	$(CXX) $(WXFLAGS) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	@rm -fr $(OBJ) $(TARGET_OUTPUT_DIR)/Resources
