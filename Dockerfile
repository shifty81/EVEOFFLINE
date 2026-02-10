# EVE OFFLINE - Dedicated Server Dockerfile
# Multi-stage build for minimal runtime image

# --- Build stage ---
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy server source and data
COPY cpp_server/ cpp_server/
COPY data/ data/

# Build server
RUN mkdir -p cpp_server/build \
    && cd cpp_server/build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DUSE_STEAM_SDK=OFF \
    && cmake --build . --config Release -j$(nproc)

# --- Runtime stage ---
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -s /bin/bash eveserver

WORKDIR /opt/eveoffline

# Copy built server binary and config
COPY --from=builder /build/cpp_server/build/bin/eve_dedicated_server .
COPY --from=builder /build/cpp_server/build/bin/config/ config/
COPY data/ data/

RUN chown -R eveserver:eveserver /opt/eveoffline

USER eveserver

EXPOSE 8765

ENTRYPOINT ["./eve_dedicated_server"]
