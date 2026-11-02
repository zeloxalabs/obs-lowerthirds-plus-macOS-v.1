# LowerThirdsPlus — build plugin for Linux and run OBS with it
# Build context: repo root (docker build -f docker/Dockerfile .)
# Stage 1: builder — OBS libobs + plugin. Stage 2: runtime — OBS + plugin.

# -----------------------------------------------------------------------------
# Stage 1: Builder (builds OBS libobs + LowerThirdsPlus plugin)
# -----------------------------------------------------------------------------
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
ENV OBS_SRC=/opt/obs-studio
ENV OBS_TAG=30.2.0

# Build system + OBS core deps (enough for libobs)
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    extra-cmake-modules \
    ninja-build \
    pkg-config \
    git \
    ca-certificates \
    libavcodec-dev \
    libavdevice-dev \
    libavfilter-dev \
    libavformat-dev \
    libavutil-dev \
    libswresample-dev \
    libswscale-dev \
    libx264-dev \
    libcurl4-openssl-dev \
    libmbedtls-dev \
    libgl1-mesa-dev \
    libjansson-dev \
    libluajit-5.1-dev \
    python3-dev \
    libx11-dev \
    libxcb-randr0-dev \
    libxcb-shm0-dev \
    libxcb-xinerama0-dev \
    libxcb-composite0-dev \
    libxcomposite-dev \
    libxinerama-dev \
    libxcb1-dev \
    libx11-xcb-dev \
    libxcb-xfixes0-dev \
    libwayland-dev \
    libglvnd-dev \
    libgles2-mesa-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# Clone OBS and build only libobs
RUN git clone --recursive --depth 1 --branch ${OBS_TAG} https://github.com/obsproject/obs-studio.git ${OBS_SRC} \
    && mkdir -p ${OBS_SRC}/build && cd ${OBS_SRC}/build \
    && cmake .. \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DENABLE_BROWSER=OFF \
        -DENABLE_SCRIPTING=OFF \
    && cmake --build . -t libobs -j$(nproc)

# Build LowerThirdsPlus plugin (source from build context = repo root)
WORKDIR /plugin
COPY CMakeLists.txt Info.plist.in ./
COPY src/ src/
COPY data/ data/
COPY obs-headers/ obs-headers/

RUN mkdir build && cd build \
    && cmake .. \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DOBS_PATH=${OBS_SRC} \
        -DOBS_BUILD_DIR=${OBS_SRC}/build \
    && cmake --build . -j$(nproc)

RUN echo "Plugin built in /plugin/build" && ls -la /plugin/build/*.so /plugin/build/*.so.* 2>/dev/null || true

VOLUME /output
CMD sh -c 'cp -v /plugin/build/*.so /output/ 2>/dev/null; ls -la /output/'

# -----------------------------------------------------------------------------
# Stage 2: Runtime — OBS Studio with Lower Thirds Plus
# -----------------------------------------------------------------------------
FROM ubuntu:22.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    software-properties-common \
    && add-apt-repository -y ppa:obsproject/obs-studio \
    && apt-get update \
    && apt-get install -y --no-install-recommends obs-studio xvfb \
    && rm -rf /var/lib/apt/lists/*

ENV OBS_PLUGIN_DIR="/root/.config/obs-studio/plugins/LowerThirdsPlus/bin/64bit"
RUN mkdir -p "$OBS_PLUGIN_DIR"

COPY --from=builder /plugin/build/*.so "$OBS_PLUGIN_DIR/"
RUN echo "=== Lower Thirds Plus plugin in image ===" && ls -la "$OBS_PLUGIN_DIR"

ENTRYPOINT ["xvfb-run", "-a", "obs"]
CMD []
