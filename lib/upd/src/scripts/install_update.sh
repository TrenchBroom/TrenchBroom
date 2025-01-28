#!/bin/bash

set -e  # Exit on error

if [ "$#" -ne 3 ] && [ "$#" -ne 4 ]; then
    echo "Usage: $0 <pid> <target_folder> <replacement_folder> [<app_to_start>]"
    exit 1
fi

PID=$1
TIMEOUT=30  # Maximum wait time in seconds
SECONDS=0   # Built-in variable that tracks script runtime

TARGET="$2"
REPLACEMENT="$3"
BACKUP="${TARGET}_backup_$(date +%s)"

if [ "$#" -eq 4 ]; then
    APP="$4"
else
    APP=""
fi

# Ensure the replacement exists
if [ ! -e "$REPLACEMENT" ]; then
    echo "Error: Replacement '$REPLACEMENT' does not exist."
    exit 1
fi

# Ensure the target exists
if [ ! -e "$TARGET" ]; then
    echo "Error: Target '$TARGET' does not exist."
    exit 1
fi

# Check if the process exists before waiting
if kill -0 "$PID" 2>/dev/null; then
echo "Waiting for process $PID to terminate (timeout: ${TIMEOUT}s)..."

while kill -0 "$PID" 2>/dev/null; do
    if [ "$SECONDS" -ge "$TIMEOUT" ]; then
        echo "Timeout reached: Process $PID is still running."
        exit 1
    fi
    sleep 1
done

echo "Process $PID has terminated."
fi

echo "Replacing '$TARGET' with '$REPLACEMENT'..."

# Move the existing target to backup folder
mkdir -p "$BACKUP"
mv "$TARGET" "$BACKUP"

# Move the replacement to the target location
mv "$REPLACEMENT" "$TARGET"

# Remove the backup folder if everything succeeded
rm -rf "$BACKUP"

echo "Replacement successful!"

if [ -n "$APP" ]; then
  # Ensure the app to start exists
  if [ ! -f "$APP" ]; then
      echo "Error: Application '$APP' does not exist."
      exit 1
  fi

  # Start the application
  echo "Starting application: $APP"
  exec "$APP"
fi
