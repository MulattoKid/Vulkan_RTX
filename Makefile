SRC_DIR = src
OBJ_DIR = build
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

GLFW_PATH = ~/glfw-3.2.1
VULKAN_SDK_PATH = ~/VulkanSDK/1.1.93.0/x86_64
#export LD_LIBRARY_PATH = $(VULKAN_SDK_PATH)/lib
#export VK_LAYER_PATH = $(VULKAN_SDK_PATH)/etc/explicit_layer.d

CXX = g++
CXXFLAGS = -std=c++11 -fopenmp -I ${GLFW_PATH}/include -I $(VULKAN_SDK_PATH)/include
LDFLAGS = -lstdc++fs -L ${GLFW_PATH}/src -lglfw3 -fopenmp -lrt -lm -ldl -lX11 -lXrandr -lXinerama -lXcursor -L $(VULKAN_SDK_PATH)/lib -lvulkan

#Setup for release
all : CXXFLAGS += -O2
all : VulkanRTX

#Setup for debug
debug : CXXFLAGS += -Wall -g -O0
debug : VulkanRTX

#Links all object files
VulkanRTX : $(OBJ_FILES)
	$(CXX) -o ./$(OBJ_DIR)/VulkanRTX $(OBJ_FILES) $(LDFLAGS)
	./CompileShaders

.PHONY : clean
clean :
	rm build/*

#Compiles all source files
#$< is the input file (.cpp file)
#$@ is the output file (.o file)
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
