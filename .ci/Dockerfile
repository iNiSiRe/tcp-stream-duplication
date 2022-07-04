FROM eu.gcr.io/biogram-dev-environment/cpp:latest

COPY ./ /app

WORKDIR /app

RUN ls -lah ./ && mkdir -p build && cd build \
    && cmake .. && make -j $(nproc) \
    && mv tcp_stream_duplicate /usr/local/bin/

ENTRYPOINT ["/usr/local/bin/tcp_stream_duplicate"]