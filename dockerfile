# Use Ubuntu base image
FROM ubuntu:22.04

# Set image metadata
LABEL maintainer="you@example.com"
LABEL description="Ubuntu-based image for PAM module development with GMP and debugging tools"

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary packages
RUN apt-get update && apt-get install -y sudo

RUN apt-get update && apt-get install -y \
    build-essential \
    libpam0g-dev \
    libgmp-dev \
    nano \
    strace \
    passwd \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Set root password to empty (INSECURE: only use in test/dev)
RUN passwd -d root

# Set working directory
WORKDIR /root/app

# Default command
CMD ["/bin/bash"]
