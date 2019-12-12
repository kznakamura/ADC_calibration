CXX      = g++
TARGET	= dummy

INC_DIR	= ./include
EX_DIR	= ./examples
OBJ_DIR	= ./obj
SRC_DIR	= ./src
BIN_DIR	= ./bin

ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs)
ROOTGLIBS = $(shell root-config --glibs)
CXXFLAGS  = -g -MMD -MP -O3 -Wall -fPIC $(ROOTCFLAGS)
CXXLIBS    = $(ROOTLIBS) $(ROOTGLIBS)

INCLUDE = -I$(INC_DIR)
SOURCES = $(wildcard $(SRC_DIR)/*.cc) 
OBJS    = $(subst $(SRC_DIR),$(OBJ_DIR), $(SOURCES:.cc=.o))
DEPENDS = $(OBJS:.o=.d)

$(BIN_DIR)/$(TARGET):	$(OBJS) $(OBJ_DIR)/$(TARGET).o
			$(CXX) $(CXXFLAGS) -o $@ $^ $(CXXLIBS)

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.cc 
	@if [ ! -d $(OBJ_DIR) ]; \
	then mkdir -p $(OBJ_DIR); \
	fi
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $< 

$(OBJ_DIR)/$(TARGET).o:	$(EX_DIR)/$(TARGET).cc
	@if [ ! -d $(OBJ_DIR) ]; \
	then mkdir -p $(OBJ_DIR); \
	fi
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	$(RM) $(OBJ_DIR)/* $(SRC_DIR)/*~ $(DEPENDS) $(BIN_DIR)/*

-include $(DEPENDS)

.PHONY:	all clean
