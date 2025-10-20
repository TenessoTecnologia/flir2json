FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential pkg-config wget unzip && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app

# Extrai o SDK da FLIR dentro do contêiner
RUN tar -xzf /app/flir_sdk/atlas-c-sdk-linux-gcc11-x64-2.14.0.tar.gz -C /app/flir_sdk

# Ajusta as variáveis pra achar as bibliotecas
ENV LD_LIBRARY_PATH=/app/flir_sdk/lib
ENV C_INCLUDE_PATH=/app/flir_sdk/include

# Compila o código
RUN gcc -O2 -Wall -Wextra \
    -I./flir_sdk/include -L./flir_sdk/lib \
    -o /app/flir2json ./src/main.c -latlas_c_sdk

# Comando de inicialização - mantém o container vivo
CMD tail -f /dev/null

