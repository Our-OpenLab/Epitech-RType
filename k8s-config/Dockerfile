FROM ubuntu:24.04 AS build

# Step 1: Install system dependencies
# Install essential tools, GCC 14, Python, and other required packages
RUN apt-get update && apt-get install -y \
    wget \
    gnupg2 \
    software-properties-common \
    build-essential \
    libssl-dev \
    python3 \
    python3-pip \
    python3-venv \
    pkg-config \
    gcc-14 \
    g++-14

# Step 2 (Optional): Install Clang (commented out for now)
# Download and execute llvm.sh to install Clang 19.1.4 (https://apt.llvm.org/)
# RUN wget https://apt.llvm.org/llvm.sh && \
#     chmod +x llvm.sh && \
#     ./llvm.sh 19 && \
#     rm llvm.sh

# Step 3: Install CMake 3.31
# Download, build, and install CMake from source
RUN wget https://github.com/Kitware/CMake/releases/download/v3.31.0/cmake-3.31.0.tar.gz && \
    tar -zxvf cmake-3.31.0.tar.gz && \
    rm cmake-3.31.0.tar.gz && \
    cd cmake-3.31.0 && \
    ./bootstrap && \
    make -j$(nproc) && \
    make install && \
    cd / && rm -rf /cmake-3.31.0

# Step 4: Set up a Python virtual environment and install Conan
# Create a virtual environment for Conan and install it
RUN python3 -m venv /opt/conan-env && \
    /opt/conan-env/bin/pip install --upgrade pip && \
    /opt/conan-env/bin/pip install conan

# Step 5: Configure environment variables
# Add the Conan virtual environment and GCC 14 to the PATH
ENV PATH="/opt/conan-env/bin:$PATH" \
    CC=/usr/bin/gcc-14 \
    CXX=/usr/bin/g++-14

# Step 6: Detect the Conan profile
# Automatically detect the profile based on the system
RUN conan profile detect

# Step 7: Prepare Conan dependencies
# Set the working directory and copy necessary files for Conan
WORKDIR /app
COPY conanfile.txt CMakeLists.txt /app/
USER root
# Install Conan dependencies (cached if conanfile.txt remains unchanged)
RUN conan install . --build=missing -c tools.system.package_manager:mode=install

# Step 8: Copy the application source code
# Copy the rest of the application files into the container
COPY . /app

# Step 9: Build the application
# Configure and build the application using CMake with Conan presets
RUN cmake --preset conan-release && \
    cmake --build --preset conan-release

# Stage 2: Final minimal image
FROM ubuntu:24.04 AS final

# Copy only the executable(s) from the build stage
WORKDIR /app
COPY --from=build /app/build/Release/bin/rtype-core /usr/local/bin/rtype-core

# Set the entrypoint to the application executable
ENTRYPOINT ["/usr/local/bin/rtype-core"]
