# Roon Knob Bridge

Lightweight Node.js bridge that exposes the `/zones`, `/now_playing`, and `/control` APIs and advertises `_roonknob._tcp` via mDNS.

## Setup

```bash
cd roon-extension
npm install
npm start
```

## Configuration

- `PORT` – HTTP listen port (default 8088).
- `LOG_LEVEL` – `debug`/`info` include more logs.
- `MDNS_NAME` – TODO: allow customizing the service name.

## Endpoints

- `GET /zones`
- `GET /now_playing?zone_id=<id>`
- `POST /control` – `{ zone_id, action, value? }`
- `GET /status`
- `GET /image` – placeholder for future image proxying.

The included `hqplayer_profile_switcher.js` file is a reference module the device team can reuse when integrating with HQPlayer.
