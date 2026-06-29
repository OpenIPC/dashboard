# Camera discovery

Dashboard combines complementary discovery methods instead of relying on one camera protocol:

1. **OpenIPC mDNS** queries `_http._tcp.local`, `_rtsp._tcp.local` and `_ssh._tcp.local`. A TXT record containing `vendor=OpenIPC` is treated as an authoritative OpenIPC match. These service definitions are shipped by the official [OpenIPC firmware](https://github.com/OpenIPC/firmware/tree/master/general/package/mdnsd-openipc/files).
2. **ONVIF WS-Discovery** sends both `NetworkVideoTransmitter` and generic probes from every selected IPv4 interface. Probes are repeated to tolerate normal UDP loss.
3. **Majestic HTTP detection** checks `/api/v1/config.json` on port 80. A structurally valid Majestic configuration is a high-confidence match. Port 85 is also checked for the legacy OpenIPC WebUI.
4. **RTSP reachability** sends a bounded `OPTIONS` probe on port 554. A generic RTSP response is shown as a lower-confidence network camera; OpenIPC/Majestic server markers raise confidence.
5. **Dahua SDK discovery** remains active and its evidence is merged with all other results by IP address.

The normal scan limits active probing to the selected interface's local `/24`. Deep scan follows the real interface prefix but is capped at `/20` (4094 usable addresses). HTTP and RTSP probes use bounded concurrency, short per-request timeouts, a global deadline and explicit cancellation.

## Compared with OpenIPC Viewer

The current Viewer discovery implementation uses ONVIF WS-Discovery with a six-second window and deduplicates Device Service URIs. Its discovery abstraction mentions future mDNS support, but the mDNS implementation is not present. Dashboard keeps correct WS-Discovery and adds firmware-native OpenIPC mDNS, Majestic/WebUI fingerprints, bounded subnet probing, progress, cancellation, confidence and cross-protocol deduplication.

- [Viewer WS-Discovery implementation](https://github.com/OpenIPC/viewer/blob/main/src/OpenIPC.Viewer.Devices/Onvif/Discovery/WsDiscoveryService.cs)
- [Viewer discovery interface](https://github.com/OpenIPC/viewer/blob/main/src/OpenIPC.Viewer.Core/Onvif/Discovery/IDiscoveryService.cs)

## Network boundary

No local discovery tool can guarantee finding devices hidden behind a routed VLAN, Wi-Fi client isolation, host firewall, disabled services or an offline camera. Multicast discovery normally stays inside one layer-2 broadcast domain, and the active sweep intentionally probes only an attached IPv4 subnet. For routed camera networks, expose discovery through network infrastructure or add an address from that subnet to the host, then select that interface in Dashboard.
