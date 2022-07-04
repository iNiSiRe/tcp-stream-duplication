# Build image

docker build -t eu.gcr.io/biogram-dev-environment/tcp_stream_duplicator:latest -f .ci/Dockerfile ./

# Push image

docker push eu.gcr.io/biogram-dev-environment/tcp_stream_duplicator:latest

# Run image

docker pull eu.gcr.io/biogram-dev-environment/tcp_stream_duplicator:latest
docker run --rm -p 5557:5557 eu.gcr.io/biogram-dev-environment/tcp_stream_duplicator:latest $(HOST) $(SOURCE_PORT) $(DEST_PORT)