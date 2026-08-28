FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y \
    build-essential cmake git \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --depth 1 --branch v1.3.2 https://github.com/USCiLab/cereal.git /tmp/cereal \
    && cp -r /tmp/cereal/include/cereal /usr/local/include/ \
    && rm -rf /tmp/cereal

WORKDIR /app
COPY . .

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j

FROM ubuntu:24.04
COPY --from=build /app/build/node /usr/local/bin/node
COPY --from=build /app/build/bootnode /usr/local/bin/bootnode
ENTRYPOINT [ "/usr/local/bin/node" ]