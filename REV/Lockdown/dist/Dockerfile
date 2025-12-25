FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    socat \
    && rm -rf /var/lib/apt/lists/*

# Create challenge directory
RUN mkdir -p /challenge

# Copy challenge binary
COPY vault_v2 /challenge/vault_v2
RUN chmod 755 /challenge/vault_v2

# Set working directory
WORKDIR /challenge

# Expose the challenge port
EXPOSE 1337

# Run with socat to handle network connections
CMD ["socat", "TCP-LISTEN:1337,reuseaddr,fork", "EXEC:/challenge/vault_v2"]
