#!/usr/bin/env bash
set -euo pipefail

IMAGE_TAG=${IMAGE_TAG:-tb-linux-ci:ubuntu22-gcc13}
WORKDIR=${WORKDIR:-$PWD}

if [[ ! -f "$WORKDIR/CI-linux.sh" ]]; then
  echo "CI-linux.sh not found in WORKDIR=$WORKDIR" >&2
  exit 1
fi

if [[ -z "${QT_ROOT_DIR:-}" ]]; then
  cat >&2 <<'EOF'
QT_ROOT_DIR is not set.
Set QT_ROOT_DIR to a Qt 6.9 installation path visible to the container.
EOF
  exit 1
fi

docker build -f docker/Dockerfile.build-linux -t "$IMAGE_TAG" "$WORKDIR"

# Derive the Qt prefix dir so we can bind-mount it if it lives outside the repo.
QT_PREFIX_DIR=$(dirname "$(dirname "$QT_ROOT_DIR")")
QT_MOUNT=()
if [[ "$QT_PREFIX_DIR" != "$WORKDIR"* ]]; then
  QT_MOUNT=(-v "$QT_PREFIX_DIR":"$QT_PREFIX_DIR")
fi

DOCKER_HOME="$WORKDIR/.docker-home"
mkdir -p "$DOCKER_HOME"

docker run --rm \
  --user "$(id -u)":"$(id -g)" \
  -e HOME="$DOCKER_HOME" \
  -e QT_ROOT_DIR="$QT_ROOT_DIR" \
  -e CCACHE_DIR="$WORKDIR/.ccache" \
  -e CC=gcc-13 \
  -e CXX=g++-13 \
  -v "$WORKDIR":"$WORKDIR" \
  "${QT_MOUNT[@]}" \
  -w "$WORKDIR" \
  "$IMAGE_TAG" \
  bash -lc './CI-linux.sh'
