# Usa imagem base do Ubuntu
FROM ubuntu:22.04

# Instala dependências
RUN apt-get update && apt-get install -y \
    build-essential pkg-config wget unzip python3 python3-pip && \
    rm -rf /var/lib/apt/lists/*

# Cria diretório da aplicação
WORKDIR /app

# Copia tudo pro container
COPY . /app

# Extrai o SDK da FLIR
RUN tar -xzf /app/flir_sdk/atlas-c-sdk-linux-gcc11-x64-2.14.0.tar.gz -C /app/flir_sdk

# Compila o programa C
RUN gcc -O2 -Wall -Wextra \
    -I./flir_sdk/include -L./flir_sdk/lib \
    -o /app/flir2json ./src/main.c -latlas_c_sdk

# Cria o servidor Python
RUN echo '\
from http.server import BaseHTTPRequestHandler, HTTPServer\n\
import json, subprocess\n\
\n\
class Handler(BaseHTTPRequestHandler):\n\
    def do_GET(self):\n\
        self.send_response(200)\n\
        self.send_header("Content-type", "application/json")\n\
        self.end_headers()\n\
        response = {"status": "ok", "message": "FLIR JSON API online!"}\n\
        self.wfile.write(json.dumps(response).encode())\n\
\n\
if __name__ == "__main__":\n\
    server = HTTPServer(("0.0.0.0", 8080), Handler)\n\
    print("🚀 Servidor rodando na porta 8080...")\n\
    server.serve_forever()\n' > /app/server.py

# Expõe a porta
EXPOSE 8080

# Inicia o servidor automaticamente
CMD ["python3", "/app/server.py"]