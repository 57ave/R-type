FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libopenal-dev \
    libflac-dev \
    libvorbis-dev \
    libogg-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libfreetype-dev \
    liblua5.4-dev \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Running the container will try to build the project
CMD ["/bin/bash"]
