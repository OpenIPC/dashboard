# Web media shared ingest / fan-out design

Status: architecture contract for P12.0; implementation is deferred to P14 and must not be
enabled before the benchmark matrix is complete.

## Problem and current baseline

The current WebRTC path owns one ingest/transcode pipeline per viewer peer. This keeps failure
and lifecycle ownership simple, but duplicate viewers of the same camera/profile repeat RTSP
ingest and may repeat decode/encode work. P12.0 paging and kiosk cycling continue to use this
known path and stop peers whenever a page becomes hidden.

## Candidate architecture

An ingest is keyed by `(camera stable ID, stream profile, transport, codec parameters)` and has
one bounded owner. Fan-out subscribers receive encoded media when passthrough is compatible or
use a bounded shared transcode branch when the browser codec requires it.

```text
camera RTSP -> ingest owner -> encoded tee -> WebRTC subscriber A
                              |            -> WebRTC subscriber B
                              +-> transcode pool -> compatible encoded tee -> subscriber C
```

Required invariants:

- camera credentials remain backend-only;
- one slow subscriber cannot block ingest or other subscribers;
- subscriber queues, transcode workers, reconnect attempts and idle grace are bounded;
- an ingest is reference-counted and stops after the last subscriber plus a short idle grace;
- page changes, logout, session revocation and socket loss release subscriptions deterministically;
- camera scope and Live View are rechecked before every subscription; Talk remains a separate,
  upstream command path and is never mixed into the media fan-out;
- metrics expose active ingests, subscribers, queue depth, dropped frames, reconnects,
  end-to-end latency and CPU/GPU/network cost without credentials or stream URLs.

## P14 benchmark gate

Compare the current per-peer implementation with passthrough fan-out and shared-transcode
candidates using 1/4/9/16/25 visible cells, 1/2/5 viewers per camera, H.264/H.265, HD/SD,
offline/reconnect storms and Windows/Linux hardware matrices. Record CPU, GPU, resident memory,
camera/network bitrate, time-to-first-frame, p50/p95 latency, dropped frames and recovery time.

The candidate may replace the baseline only when it:

- reduces duplicate camera traffic and aggregate resource cost for multi-viewer cases;
- does not regress single-viewer latency or recovery beyond an agreed tolerance;
- remains bounded under stalled subscribers and reconnect storms;
- passes camera-scope/session-revocation tests and clean shutdown/leak checks.

Until those results exist, `DashboardWebRtcManager` remains the production owner and no hidden
shared-ingest cache or long-lived background pipeline is introduced.
