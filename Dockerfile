# Use a lighter base image
FROM debian:bookworm-slim as builder

# Define build argument for configuration
ARG BUILD_TYPE=Debug

# Install dependencies in a single layer and clean up after
RUN apt-get update && apt-get install -y \
  build-essential cmake curl freeglut3-dev gcc git libassimp-dev \
  libfreeimage-dev libfreetype6-dev libglew-dev libgl1-mesa-dev \
  libglm-dev libglu1-mesa-dev libqt5svg5-dev libtinyxml2-dev libxi-dev \
  libxrandr-dev libxxf86vm-dev mesa-common-dev ninja-build pandoc \
  p7zip-full pkg-config qtbase5-dev tar unzip xvfb zip \
  && rm -rf /var/lib/apt/lists/*

# Clone the local TrenchBroom directory and switch to it
COPY . /TrenchBroom
WORKDIR /TrenchBroom

# Create a build directory and switch to it
RUN mkdir build
WORKDIR /TrenchBroom/build

# Build TrenchBroom
RUN cmake .. -GNinja -DCMAKE_TOOLCHAIN_FILE=/TrenchBroom/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_PREFIX_PATH="cmake/packages" -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_CXX_FLAGS="-Werror" \
  -DCMAKE_EXE_LINKER_FLAGS="-Wl,--fatal-warnings" -DTB_SUPPRESS_PCH=1 \
  && cmake --build . --config ${BUILD_TYPE}
