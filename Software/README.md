# Software

Dashboard, server, and helper tools for Cube OS telepresence.

No dashboard app source is committed yet. `src/` and `screenshots/` are placeholders only.

## Dashboard Scope

- Pairing, identity, online/offline presence
- Chat, voice notes, sounds, and display reactions
- Safe remote control: walk, turn, wiggle, sleep, wake
- Camera snapshots/clips and telemetry views
- Quiet hours, rest mode, privacy, and notification limits

## Likely Stack

- Frontend: React/Next.js or SvelteKit
- Backend: Node.js WebSocket server or FastAPI
- Protocol: WebSocket or MQTT with command ACKs
- Media: JPEG snapshots or short MJPEG previews first
