#!/bin/bash

# Exit script on error
set -e

# Default build type is Debug
BUILD_TYPE=${1:-Debug}

# Convert build type to lowercase to handle both 'Debug' and 'debug'
BUILD_TYPE_LOWER=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')

# Define image and container names based on the build type
IMAGE_NAME="trenchbroom-docker-${BUILD_TYPE_LOWER}"
CONTAINER_NAME="${IMAGE_NAME}-container-$(date +%Y%m%d%H%M%S)"

# Build the Docker image
DOCKER_BUILDKIT=1 docker build -t $IMAGE_NAME --build-arg BUILD_TYPE=$BUILD_TYPE .

# Run the container
docker run -d --name $CONTAINER_NAME $IMAGE_NAME

# Delete local app directory
rm -rf build/app/

# Ensure local build directory exists, create it if it does not
mkdir -p build/

# Copy files from the container to the host
docker cp $CONTAINER_NAME:/TrenchBroom/build/app/ build/app/

# Stop and remove the container
docker stop $CONTAINER_NAME
docker rm $CONTAINER_NAME

echo "Linux build and copy for $BUILD_TYPE successfully completed."
