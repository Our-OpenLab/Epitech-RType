#!/bin/bash

# Colors
RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
CYAN="\033[0;36m"
RESET="\033[0m"

# Variables
# IMAGE_NAME="guillaumemichel1026/rtype"
# IMAGE_TAG="latest"
# DOCKERFILE_PATH="Dockerfile" # Path to the Dockerfile

# Variables
IMAGE_NAME="guillaumemichel1026/rtype-dev"
IMAGE_TAG="latest"
DOCKERFILE_PATH="Dockerfile.dev"

# Build the Docker image
echo -e "${YELLOW}Building the Docker image...${RESET}"
if ! docker build --progress=plain -t "$IMAGE_NAME:$IMAGE_TAG" -f "$DOCKERFILE_PATH" ..; then
    echo -e "${RED}Error: Failed to build the Docker image.${RESET}"
    exit 1
fi
echo -e "${GREEN}Docker image built successfully: ${IMAGE_NAME}:${IMAGE_TAG}${RESET}"

# Push the image to Docker Hub
echo -e "${YELLOW}Pushing the Docker image to Docker Hub...${RESET}"
if ! docker push "$IMAGE_NAME:$IMAGE_TAG"; then
    echo -e "${RED}Error: Failed to push the Docker image.${RESET}"
    exit 1
fi
echo -e "${GREEN}Docker image pushed successfully to Docker Hub: ${IMAGE_NAME}:${IMAGE_TAG}${RESET}"

# Script completed
echo -e "${CYAN}Script completed successfully!${RESET}"
