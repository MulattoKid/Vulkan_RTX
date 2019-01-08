SRC_DIR = src
VOLK_DIR = $(SRC_DIR)/volk
OBJ_DIR = build

GLFW_PATH = ~/glfw-3.2.1
VULKAN_SDK_PATH = ~/VulkanSDK/1.1.92.1/x86_64
#export LD_LIBRARY_PATH = $(VULKAN_SDK_PATH)/lib
#export VK_LAYER_PATH = $(VULKAN_SDK_PATH)/etc/explicit_layer.d

CXX = g++
CXXFLAGS = -std=c++11 -fopenmp -I $(GLFW_PATH)/include -I $(VULKAN_SDK_PATH)/include
LDFLAGS = -lstdc++fs -L $(GLFW_PATH)/src -lglfw3 -fopenmp -lrt -lm -ldl -lX11 -L $(VULKAN_SDK_PATH)/lib -lXrandr -lXinerama -lXcursor

#Setup for release
all : CXXFLAGS += -O2
all : VulkanRTX

#Setup for debug
debug : CXXFLAGS += -Wall -g -O0
debug : VulkanRTX

VulkanRTX :
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/Logger.cpp -o $(OBJ_DIR)/Logger.o
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/RNG.cpp -o $(OBJ_DIR)/RNG.o
	$(CXX) $(CXXFLAGS) -c $(VOLK_DIR)/volk.c -o $(OBJ_DIR)/volk.o
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/VulkanApp.cpp -o $(OBJ_DIR)/VulkanApp.o
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/BrhanFile.cpp -o $(OBJ_DIR)/BrhanFile.o
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp -o $(OBJ_DIR)/main.o
	$(CXX) -o $(OBJ_DIR)/VulkanRTX $(OBJ_DIR)/main.o $(OBJ_DIR)/BrhanFile.o $(OBJ_DIR)/VulkanApp.o $(OBJ_DIR)/volk.o $(OBJ_DIR)/RNG.o $(OBJ_DIR)/Logger.o $(LDFLAGS)
	./CompileShaders

.PHONY : clean
clean :
	rm build/*
