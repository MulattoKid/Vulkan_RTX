SRC_DIR = src
VOLK_DIR = $(SRC_DIR)/volk
OBJ_DIR = build

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))
OBJ_FILES += $(OBJ_DIR)/volk.o

GLFW_PATH = ~/glfw-3.2.1
VULKAN_SDK_PATH = ~/VulkanSDK/1.1.92.1/x86_64

CXX = g++
CXXFLAGS = -std=c++11 -I $(GLFW_PATH)/include -I $(VULKAN_SDK_PATH)/include
LDFLAGS = -lstdc++fs -L $(VULKAN_SDK_PATH)/lib -L $(GLFW_PATH)/src -fopenmp -lglfw3 -lrt -lm -ldl -lX11 -lXrandr -lXinerama -lXcursor

#Setup for release
all : CXXFLAGS += -O2
all : VulkanRTX

#Setup for debug
debug : CXXFLAGS += -Wall -g -O0
debug : VulkanRTX

#VulkanRTX :
#	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/Logger.cpp -o $(OBJ_DIR)/Logger.o
#	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/RNG.cpp -o $(OBJ_DIR)/RNG.o
#	$(CXX) $(CXXFLAGS) -c $(VOLK_DIR)/volk.c -o $(OBJ_DIR)/volk.o
#	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/Camera.cpp -o $(OBJ_DIR)/Camera.o
#	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/VulkanApp.cpp -o $(OBJ_DIR)/VulkanApp.o
#	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/BrhanFile.cpp -o $(OBJ_DIR)/BrhanFile.o
#	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp -o $(OBJ_DIR)/main.o
#	$(CXX) -o $(OBJ_DIR)/VulkanRTX $(OBJ_DIR)/main.o $(OBJ_DIR)/BrhanFile.o $(OBJ_DIR)/VulkanApp.o $(OBJ_DIR)/Camera.o $(OBJ_DIR)/volk.o $(OBJ_DIR)/RNG.o $(OBJ_DIR)/Logger.o $(LDFLAGS)
VulkanRTX : $(OBJ_FILES)
	$(CXX) -o ./$(OBJ_DIR)/VulkanRTX $(OBJ_FILES) $(LDFLAGS)
	./CompileShaders
	
#Compiles all source files
#$< is the input file (.cpp file)
#$@ is the output file (.o file)
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(OBJ_DIR)/volk.o : $(VOLK_DIR)/volk.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY : clean
clean :
	rm build/*
	rm src/shaders/out/*.spv
	rm src/shaders/out/color_position/*.spv
	rm src/shaders/out/ao/*.spv
