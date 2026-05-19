#!/bin/bash
set -e

BUILD_DIR="build-web"
PORT="${PORT:-8000}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Source file extensions to watch
WATCH_EXTS="c|cpp|cc|cxx|h|hpp|cmake|CMakeLists.txt"

# Initial build
echo "[watch] Initial build..."
emcmake cmake -B "$BUILD_DIR"
cmake --build "$BUILD_DIR"

# Start HTTP server in background
echo "[watch] Starting HTTP server on port $PORT..."
python3 "$SCRIPT_DIR/serve.py" "$BUILD_DIR" "$PORT" &
SERVER_PID=$!

cleanup() {
  echo ""
  echo "[watch] Shutting down..."
  kill "$SERVER_PID" 2>/dev/null
  exit 0
}
trap cleanup INT TERM

echo "[watch] Watching for changes... (Ctrl+C to stop)"

get_mtime() {
  {
    # Source files
    find . \
      -not -path "./$BUILD_DIR/*" \
      -not -path "./.git/*" \
      | grep -E "\.(${WATCH_EXTS})$"

    # Resources folder (all files, any extension)
    if [ -d "./resources" ]; then
      find ./resources -type f
    fi
  } | xargs stat -f "%m" 2>/dev/null \
    || xargs stat -c "%Y" 2>/dev/null \
    | sort -n | tail -1
}

LAST_MTIME=$(get_mtime)

while true; do
  sleep 1

  CURRENT_MTIME=$(get_mtime)

  if [ "$CURRENT_MTIME" != "$LAST_MTIME" ]; then
    echo "[watch] Change detected, rebuilding..."
    if cmake --build "$BUILD_DIR"; then
      echo "[watch] Build succeeded — reloading browsers..."
      curl -s "http://localhost:$PORT/__trigger-reload" > /dev/null || true
    else
      echo "[watch] Build FAILED — server still running with last good build."
    fi
    LAST_MTIME=$(get_mtime)
  fi
done