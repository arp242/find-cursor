# Build stage
FROM debian:10 as builder

# Install dependencies
RUN apt update && \
    apt install --yes \
        build-essential \
        libx11-dev libxcomposite-dev libxdamage-dev libxrender-dev \
        git

# Copy source code into container
COPY . /src

# Build
WORKDIR /src
RUN mkdir --parents /usr/local/share/man/man1/ && \
    make && make install




# Final image
FROM debian:10

# Copy the compiled binary across
COPY --from=builder /usr/local/bin/find-cursor /usr/local/bin/find-cursor

# Install dependent libs.
RUN apt update && \
    apt install --yes --no-install-recommends \
        libx11-6 libxext6 libxfixes3

# Run
CMD ["/usr/local/bin/find-cursor", "--help"]
