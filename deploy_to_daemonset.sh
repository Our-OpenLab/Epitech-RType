#!/bin/bash

# Colors for output
RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
CYAN="\033[0;36m"
RESET="\033[0m"

# Variables
LOCAL_CODE_DIR="/Users/guillaume/Epitech/teck3/Epitech-RType/." # Update with your local code directory
DAEMONSET_LABEL="app=server" # Update with your DaemonSet's label selector
APP_PATH_IN_POD="/app"

# Step 1: Get all pods from the DaemonSet
echo -e "${YELLOW}Retrieving pods from the DaemonSet...${RESET}"
POD_NAMES=$(kubectl get pods -l "$DAEMONSET_LABEL" -o jsonpath="{.items[*].metadata.name}")

if [ -z "$POD_NAMES" ]; then
    echo -e "${RED}No pods found for DaemonSet with label: $DAEMONSET_LABEL.${RESET}"
    exit 1
fi

# Function to process a single pod
process_pod() {
    local POD_NAME=$1
    echo -e "${CYAN}Processing pod: $POD_NAME${RESET}"

    # Step 2: Copy local code to the pod
    echo -e "${YELLOW}Copying local code to the pod, excluding CMakeUserPresets.json...${RESET}"
    TEMP_ARCHIVE="/tmp/code_to_copy_$POD_NAME.tar"
    tar --exclude="CMakeUserPresets.json" -cf "$TEMP_ARCHIVE" -C "$LOCAL_CODE_DIR" .

    if ! kubectl cp "$TEMP_ARCHIVE" "$POD_NAME:/tmp/code_to_copy.tar"; then
        echo -e "${RED}Failed to copy code archive to the pod: $POD_NAME.${RESET}"
        rm -f "$TEMP_ARCHIVE"
        return 1
    fi

    if ! kubectl exec "$POD_NAME" -- tar -xf /tmp/code_to_copy.tar -C "$APP_PATH_IN_POD"; then
        echo -e "${RED}Failed to extract code inside the pod: $POD_NAME.${RESET}"
        kubectl exec "$POD_NAME" -- rm -f /tmp/code_to_copy.tar
        rm -f "$TEMP_ARCHIVE"
        return 1
    fi

    kubectl exec "$POD_NAME" -- rm -f /tmp/code_to_copy.tar
    rm -f "$TEMP_ARCHIVE"
    echo -e "${GREEN}Code copied successfully to $POD_NAME.${RESET}"

    # Step 3: Install dependencies with Conan inside the Pod
    echo -e "${YELLOW}Installing dependencies with Conan in the pod: $POD_NAME...${RESET}"
    if ! kubectl exec "$POD_NAME" -- bash -c "cd $APP_PATH_IN_POD && conan install . --build=missing -c tools.system.package_manager:mode=install"; then
        echo -e "${RED}Failed to install dependencies with Conan in pod: $POD_NAME.${RESET}"
        return 1
    fi
    echo -e "${GREEN}Dependencies installed successfully in pod: $POD_NAME.${RESET}"

    # Step 4: Configure and build the application with CMake
    echo -e "${YELLOW}Configuring and building the application in pod: $POD_NAME...${RESET}"
    if ! kubectl exec "$POD_NAME" -- bash -c "cd $APP_PATH_IN_POD && cmake --preset conan-release && cmake --build --preset conan-release"; then
        echo -e "${RED}Failed to configure and build the application in pod: $POD_NAME.${RESET}"
        return 1
    fi
    echo -e "${GREEN}Application configured and built successfully in pod: $POD_NAME.${RESET}"

    # Step 5: Run the application
    echo -e "${YELLOW}Running the application in pod: $POD_NAME...${RESET}"
    if ! kubectl exec "$POD_NAME" -- bash -c "/app/build/Release/bin/RTypeCore"; then
        echo -e "${RED}Failed to run the application in pod: $POD_NAME.${RESET}"
        return 1
    fi
    echo -e "${GREEN}Application is running in pod: $POD_NAME.${RESET}"
}

# Run the function in parallel for all pods
echo -e "${YELLOW}Starting tasks for all pods in parallel...${RESET}"
for POD_NAME in $POD_NAMES; do
    process_pod "$POD_NAME" &
done

# Wait for all background jobs to finish
wait

echo -e "${GREEN}All tasks completed for all pods.${RESET}"
