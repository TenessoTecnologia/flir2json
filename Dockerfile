FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential pkg-config wget unzip python3 && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app

# extrai SDK (já está no diretório flir_sdk)
RUN tar -xzf /app/flir_sdk/atlas-c-sdk-linux-gcc11-x64-2.14.0.tar.gz -C /app/flir_sdk

# compila o extrator (novo) + mantém o flir2json antigo se quiser
RUN gcc -O2 -Wall -Wextra \
    -I./flir_sdk/include -L./flir_sdk/lib \
    -o /app/extract ./src/extract.c -latlas_c_sdk && \
    gcc -O2 -Wall -Wextra \
    -I./flir_sdk/include -L./flir_sdk/lib \
    -o /app/flir2json ./src/main.c -latlas_c_sdk || true

EXPOSE 8080
CMD ["python3", "/app/server.py"]