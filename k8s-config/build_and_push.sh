#!/bin/bash

# Colors
RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
CYAN="\033[0;36m"
RESET="\033[0m"

# Variables
IMAGE_NAME="guillaumemichel1026/rtype-dev"
# IMAGE_NAME="guillaumemichel1026/rtype-test"
IMAGE_TAG="latest"
DOCKERFILE_PATH="Dockerfile.dev" # Path to the Dockerfile
# DOCKERFILE_PATH="Dockerfile.test"

Build the Docker image
echo -e "${YELLOW}Building the Docker image${RESET}"
if ! docker buildx build --progress=plain -t "$IMAGE_NAME:$IMAGE_TAG" -f "$DOCKERFILE_PATH" --push ..; then
   echo -e "${RED}Error: Failed to build and push the Docker image.${RESET}"
   exit 1
fi
echo -e "${GREEN}Docker image built and pushed successfully: ${IMAGE_NAME}:${IMAGE_TAG}${RESET}"

# Script completed
echo -e "${CYAN}Script completed successfully!${RESET}"
