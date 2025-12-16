FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build tools and dependencies
# We install:
# - build-essential (gcc, g++, make)
# - cmake
# - git
# - libsfml-dev (SFML dependencies)
# - libasio-dev (Asio standalone)
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libsfml-dev \
    libasio-dev \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Running the container will try to build the project
CMD ["/bin/bash"]
