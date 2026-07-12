# Camera discovery

Dashboard combines complementary discovery methods instead of relying on one camera protocol:

1. **OpenIPC mDNS** queries `_http._tcp.local`, `_rtsp._tcp.local` and `_ssh._tcp.local`. A TXT record containing `vendor=OpenIPC` is treated as an authoritative OpenIPC match. These service definitions are shipped by the official [OpenIPC firmware](https://github.com/OpenIPC/firmware/tree/master/general/package/mdnsd-openipc/files).
2. **ONVIF WS-Discovery** sends both `NetworkVideoTransmitter` and generic probes from every selected IPv4 interface. Probes are repeated to tolerate normal UDP loss.
3. **Majestic HTTP detection** checks `/api/v1/config.json` on port 80. A structurally valid Majestic configuration is a high-confidence match. Port 85 is also checked for the legacy OpenIPC WebUI.
4. **RTSP reachability** sends a bounded `OPTIONS` probe on port 554. A generic RTSP response is shown as a lower-confidence network camera; OpenIPC/Majestic server markers raise confidence.
5. **Dahua SDK discovery** remains active and its evidence is merged with all other results by IP address.

The normal scan limits active probing to the selected interface's local `/24`. Deep scan follows the real interface prefix but is capped at `/20` (4094 usable addresses). HTTP and RTSP probes use bounded concurrency, short per-request timeouts, a global deadline and explicit cancellation.

## Compared with OpenIPC Viewer

As of 2026-07-04, OpenIPC Viewer has moved to a source-based discovery pipeline with ONVIF, mDNS and an opt-in subnet sweep. Dashboard should no longer claim that Viewer lacks mDNS. The remaining Dashboard distinction is the OpenIPC-control-center bias: discovery evidence is merged with Majestic/WebUI fingerprints, RTSP probes and Dahua SDK results, then carried into onboarding, health/status and OpenIPC/Majestic tooling with confidence and evidence text.

- [Viewer WS-Discovery implementation](https://github.com/OpenIPC/viewer/blob/main/src/OpenIPC.Viewer.Devices/Onvif/Discovery/WsDiscoveryService.cs)
- [Viewer discovery aggregator](https://github.com/OpenIPC/viewer/blob/main/src/OpenIPC.Viewer.Devices/Discovery/DiscoveryAggregator.cs)
- [Viewer mDNS source](https://github.com/OpenIPC/viewer/blob/main/src/OpenIPC.Viewer.Devices/Discovery/MdnsDiscoverySource.cs)
- [Viewer subnet sweep source](https://github.com/OpenIPC/viewer/blob/main/src/OpenIPC.Viewer.Devices/Discovery/SubnetSweepDiscoverySource.cs)

## Network boundary

No local discovery tool can guarantee finding devices hidden behind a routed VLAN, Wi-Fi client isolation, host firewall, disabled services or an offline camera. Multicast discovery normally stays inside one layer-2 broadcast domain, and the active sweep intentionally probes only an attached IPv4 subnet. For routed camera networks, expose discovery through network infrastructure or add an address from that subnet to the host, then select that interface in Dashboard.
