#!/usr/bin/env bash
set -euo pipefail

container_name="openipc-mediamtx-smoke"
publisher_log="${RUNNER_TEMP:-/tmp}/openipc-rtsp-publisher.log"

cleanup() {
  if [[ -n "${publisher_pid:-}" ]]; then
    kill "$publisher_pid" 2>/dev/null || true
  fi
  docker rm -f "$container_name" >/dev/null 2>&1 || true
}
trap cleanup EXIT

docker run --detach --rm --name "$container_name" \
  --publish 8554:8554 bluenviron/mediamtx:1.19.1 >/dev/null

for _ in $(seq 1 30); do
  if docker logs "$container_name" 2>&1 | grep -q "listener opened"; then
    break
  fi
  sleep 0.25
done

ffmpeg -hide_banner -loglevel warning -re \
  -f lavfi -i "testsrc=size=640x360:rate=15" \
  -an -c:v libx264 -preset ultrafast -tune zerolatency \
  -f rtsp -rtsp_transport tcp rtsp://127.0.0.1:8554/openipc-smoke \
  >"$publisher_log" 2>&1 &
publisher_pid=$!

for _ in $(seq 1 30); do
  if docker logs "$container_name" 2>&1 | grep -q "openipc-smoke.*is publishing"; then
    break
  fi
  if ! kill -0 "$publisher_pid" 2>/dev/null; then
    cat "$publisher_log"
    exit 1
  fi
  sleep 0.25
done

timeout 20 gst-launch-1.0 -q -e \
  rtspsrc location=rtsp://127.0.0.1:8554/openipc-smoke protocols=tcp latency=100 \
  ! rtph264depay ! h264parse ! identity eos-after=30 ! fakesink sync=false
