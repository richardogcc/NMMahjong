# Imagen para probar el ejecutable de Windows con Wine, sin pantalla ni sonido.
FROM debian:bookworm-slim
RUN apt-get update \
 && apt-get install -y --no-install-recommends wine wine64 xvfb xauth libgl1-mesa-dri \
 && rm -rf /var/lib/apt/lists/*
ENV WINEDEBUG=-all WINEPREFIX=/tmp/wine LIBGL_ALWAYS_SOFTWARE=1
WORKDIR /game
