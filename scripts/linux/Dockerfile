# Imagen para compilar y probar la versión de Linux (x86-64).
# Ubuntu 22.04: el binario funciona en cualquier distribución con glibc 2.35 o posterior.
FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends ca-certificates wget gnupg build-essential ninja-build git \
      libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev libasound2-dev \
      xvfb xauth libgl1-mesa-dri \
 && wget -qO- https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor -o /usr/share/keyrings/kitware.gpg \
 && echo 'deb [signed-by=/usr/share/keyrings/kitware.gpg] https://apt.kitware.com/ubuntu/ jammy main' > /etc/apt/sources.list.d/kitware.list \
 && apt-get update && apt-get install -y --no-install-recommends cmake \
 && rm -rf /var/lib/apt/lists/*
ENV LIBGL_ALWAYS_SOFTWARE=1
WORKDIR /src
