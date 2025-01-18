#!/bin/bash

# Colors for output
RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
CYAN="\033[0;36m"
RESET="\033[0m"

# Variables
LOCAL_CODE_DIR="/Users/guillaume/Epitech/teck3/Epitech-RType/." # Update with your local code directory
POD_NAME="server-pod"
APP_PATH_IN_POD="/app"

# Step 1: Copy local code to the Pod
#echo -e "${YELLOW}Copying local code to the pod...${RESET}"
#if ! kubectl cp "$LOCAL_CODE_DIR" "$POD_NAME:$APP_PATH_IN_POD"; then
#    echo -e "${RED}Failed to copy code to the pod.${RESET}"
#    exit 1
#fi
#echo -e "${GREEN}Code copied successfully to ${POD_NAME}:${APP_PATH_IN_POD}.${RESET}"


echo -e "${YELLOW}Copying local code to the pod, excluding CMakeUserPresets.json...${RESET}"

# Create a temporary tar archive excluding the unwanted file
TEMP_ARCHIVE="/tmp/code_to_copy.tar"
tar --exclude="CMakeUserPresets.json" -cf "$TEMP_ARCHIVE" -C "$LOCAL_CODE_DIR" .

if ! kubectl cp "$TEMP_ARCHIVE" "$POD_NAME:/tmp/code_to_copy.tar"; then
    echo -e "${RED}Failed to copy code archive to the pod.${RESET}"
    rm -f "$TEMP_ARCHIVE"
    exit 1
fi

# Extract the archive inside the pod
if ! kubectl exec "$POD_NAME" -- tar -xf /tmp/code_to_copy.tar -C "$APP_PATH_IN_POD"; then
    echo -e "${RED}Failed to extract code inside the pod.${RESET}"
    kubectl exec "$POD_NAME" -- rm -f /tmp/code_to_copy.tar
    rm -f "$TEMP_ARCHIVE"
    exit 1
fi

# Clean up temporary files
kubectl exec "$POD_NAME" -- rm -f /tmp/code_to_copy.tar
rm -f "$TEMP_ARCHIVE"

echo -e "${GREEN}Code copied successfully to ${POD_NAME}:${APP_PATH_IN_POD}, excluding CMakeUserPresets.json.${RESET}"



# Step 2: Install dependencies with Conan inside the Pod
echo -e "${YELLOW}Installing dependencies with Conan in the pod...${RESET}"
if ! kubectl exec "$POD_NAME" -- bash -c "cd $APP_PATH_IN_POD && conan install . --build=missing -c tools.system.package_manager:mode=install"; then
    echo -e "${RED}Failed to install dependencies with Conan.${RESET}"
    exit 1
fi
echo -e "${GREEN}Dependencies installed successfully.${RESET}"

# Step 3: Configure and build the application with CMake
echo -e "${YELLOW}Configuring and building the application with CMake...${RESET}"
if ! kubectl exec "$POD_NAME" -- bash -c "cd $APP_PATH_IN_POD && cmake --preset conan-release && cmake --build --preset conan-release"; then
    echo -e "${RED}Failed to configure and build the application.${RESET}"
    exit 1
fi
echo -e "${GREEN}Application configured and built successfully.${RESET}"

# Step 4: Run the application
echo -e "${YELLOW}Running the application in the pod...${RESET}"
if ! kubectl exec "$POD_NAME" -- bash -c "valgrind --leak-check=full /app/build/Release/bin/RTypeCore"; then
    echo -e "${RED}Failed to run the application.${RESET}"
    exit 1
fi
echo -e "${GREEN}Application is running.${RESET}"
