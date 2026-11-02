# Lower Thirds Plus — Docker

Build the plugin and run OBS with it **inside Docker**. No need to install OBS or “add source” on your host — everything runs in the container.

## Build and run (one flow)

From the **repo root**:

```bash
./docker/build-and-run.sh
```

This builds the Docker image (first time ~10–20 min) and then runs OBS with Lower Thirds Plus loaded. The plugin is built from the current source and included in the image.

## Manual steps

**Build the image (from repo root):**

```bash
docker build -f docker/Dockerfile -t lowerthirdsplus .
```

**Run OBS with the plugin:**

```bash
docker run -it --rm lowerthirdsplus
```

**Only extract the plugin .so (no OBS in container):**

```bash
docker build -f docker/Dockerfile --target builder -t lowerthirdsplus-builder .
mkdir -p output
docker run --rm -v "$(pwd)/output:/output" lowerthirdsplus-builder
# Plugin is in ./output/
```

**Show OBS GUI on your machine (X11 / XQuartz):**

```bash
docker run -it --rm -e DISPLAY=host.docker.internal:0 -e QT_QPA_PLATFORM=xcb lowerthirdsplus
```

## docker-compose

From repo root:

```bash
docker compose -f docker/docker-compose.yml build
docker compose -f docker/docker-compose.yml run --rm obs
```
