#!/bin/bash

set -e  # Exit on any error
set -u  # Treat undefined variables as errors

# Colors for success, warning, and error
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No color

echo -e "${YELLOW}=== Starting installation and build on Linux/macOS ===${NC}"

# Step 1: Check for Conan
echo -e "${YELLOW}Checking for Conan...${NC}"
if ! command -v conan &> /dev/null; then
    echo -e "${RED}ERROR: Conan is not installed. Please install it and try again.${NC}"
    exit 1
else
    echo -e "${GREEN}SUCCESS: Conan is installed.${NC}"
fi

# Step 2: Check for CMake
echo -e "${YELLOW}Checking for CMake...${NC}"
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}ERROR: CMake is not installed. Please install it and try again.${NC}"
    exit 1
else
    echo -e "${GREEN}SUCCESS: CMake is installed.${NC}"
fi

# Step 3: Create the build directory
BUILD_DIR="build"
echo -e "${YELLOW}Creating the build directory...${NC}"
if [ ! -d "$BUILD_DIR" ]; then
#    mkdir "$BUILD_DIR"
    echo -e "${GREEN}SUCCESS: Build directory created.${NC}"
else
    echo -e "${GREEN}SUCCESS: Build directory already exists.${NC}"
fi

# Step 4: Install dependencies with Conan
echo -e "${YELLOW}Installing dependencies with Conan...${NC}"
if conan install . --output-folder="$BUILD_DIR" --build=missing; then
    echo -e "${GREEN}SUCCESS: Dependencies installed successfully.${NC}"
else
    echo -e "${RED}ERROR: Conan failed to install dependencies.${NC}"
    exit 1
fi

# Step 5: Configure the project with CMake
echo -e "${YELLOW}Configuring the project with CMake...${NC}"
cd "$BUILD_DIR"
if cmake .. -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release; then
    echo -e "${GREEN}SUCCESS: CMake configuration completed.${NC}"
else
    echo -e "${RED}ERROR: CMake configuration failed.${NC}"
    exit 1
fi

# Step 6: Build the project
echo -e "${YELLOW}Building the project...${NC}"
if cmake --build .; then
    echo -e "${GREEN}SUCCESS: Project built successfully.${NC}"
else
    echo -e "${RED}ERROR: Build failed.${NC}"
    exit 1
fi

echo -e "${GREEN}=== Installation and build completed successfully! ===${NC}"
exit 0
