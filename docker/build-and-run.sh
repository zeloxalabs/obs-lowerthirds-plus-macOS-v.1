#!/bin/bash
# Build the plugin as Docker image and run OBS with Lower Thirds Plus inside the container.
# Run from repo root: ./docker/build-and-run.sh
# No "add source" on your host — OBS runs in Docker with the plugin already loaded.

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
IMAGE="${IMAGE:-lowerthirdsplus}"

cd "$REPO_ROOT"
echo "Building Docker image (first time: 10–20 min)..."
docker build -f docker/Dockerfile -t "$IMAGE" .

echo ""
echo "Running OBS with Lower Thirds Plus in Docker..."
echo "Inside the container, OBS has the plugin; add 'Lower Thirds Plus Source' there if you need it."
echo ""
docker run -it --rm "$IMAGE"
