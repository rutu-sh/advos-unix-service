# Dockerfile
FROM ubuntu:22.04

# Install necessary packages
RUN apt-get update && \
    apt-get install -y build-essential linux-headers-generic libyaml-dev gcc g++ gdb && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


# RUN git clone https://github.com/rutu-sh/cloudlab-tools.git && cd cloudlab-tools/tools/generic && make install-go

# Pause script to keep the container running
CMD ["sh", "-c", "tail -f /dev/null"]
