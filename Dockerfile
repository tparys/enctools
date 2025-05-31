FROM ubuntu:jammy as enctools-dev
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential cmake libcairo2-dev libcgal-dev libcgal-qt5-dev libgdal-dev libgtest-dev libmicrohttpd-dev libtinyxml2-dev

FROM enctools-dev as enctools-compile
WORKDIR /build
RUN --mount=type=bind,source=.,target=/source \
    cmake /source && \
    make -j$(nproc) && \
    make test
