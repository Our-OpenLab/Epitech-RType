# Stop the script on any error
$ErrorActionPreference = "Stop"

# Colors for success, warning, and error
$GREEN = "`e[32m"
$YELLOW = "`e[33m"
$RED = "`e[31m"
$NC = "`e[0m"

Write-Host "${YELLOW}=== Starting installation and build on Windows ===${NC}"

# Step 1: Check for Conan
Write-Host "${YELLOW}Checking for Conan...${NC}"
if (-not (Get-Command "conan" -ErrorAction SilentlyContinue)) {
    Write-Host "${RED}ERROR: Conan is not installed. Please install it and try again.${NC}"
    exit 1
} else {
    Write-Host "${GREEN}SUCCESS: Conan is installed.${NC}"
}

# Step 2: Check for CMake
Write-Host "${YELLOW}Checking for CMake...${NC}"
if (-not (Get-Command "cmake" -ErrorAction SilentlyContinue)) {
    Write-Host "${RED}ERROR: CMake is not installed. Please install it and try again.${NC}"
    exit 1
} else {
    Write-Host "${GREEN}SUCCESS: CMake is installed.${NC}"
}

# Step 3: Create the build directory
$BUILD_DIR = "build"
Write-Host "${YELLOW}Creating the build directory...${NC}"
if (-not (Test-Path $BUILD_DIR)) {
    New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null
    Write-Host "${GREEN}SUCCESS: Build directory created.${NC}"
} else {
    Write-Host "${GREEN}SUCCESS: Build directory already exists.${NC}"
}

# Step 4: Install dependencies with Conan
Write-Host "${YELLOW}Installing dependencies with Conan...${NC}"
if (conan install . --output-folder=$BUILD_DIR --build=missing) {
    Write-Host "${GREEN}SUCCESS: Dependencies installed successfully.${NC}"
} else {
    Write-Host "${RED}ERROR: Conan failed to install dependencies.${NC}"
    exit 1
}

# Step 5: Configure the project with CMake
Write-Host "${YELLOW}Configuring the project with CMake...${NC}"
cd $BUILD_DIR
if (cmake .. -DCMAKE_TOOLCHAIN_FILE="build/Release/generators/conan_toolchain.cmake" -DCMAKE_BUILD_TYPE=Release) {
    Write-Host "${GREEN}SUCCESS: CMake configuration completed.${NC}"
} else {
    Write-Host "${RED}ERROR: CMake configuration failed.${NC}"
    exit 1
}

# Step 6: Build the project
Write-Host "${YELLOW}Building the project...${NC}"
if (cmake --build .) {
    Write-Host "${GREEN}SUCCESS: Project built successfully.${NC}"
} else {
    Write-Host "${RED}ERROR: Build failed.${NC}"
    exit 1
}

Write-Host "${GREEN}=== Installation and build completed successfully! ===${NC}"
exit 0
