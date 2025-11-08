const express = require('express');
const { advertise } = require('./mdns');

const app = express();
const PORT = parseInt(process.env.PORT || '8088', 10);
const LOG_LEVEL = process.env.LOG_LEVEL || 'info';
const BRIDGE_BASE = `http://localhost:${PORT}`;

const zones = [
  { zone_id: 'office', zone_name: 'Office' },
  { zone_id: 'main', zone_name: 'Main Room' }
];

const nowPlayingStore = {
  line1: 'Roon Knob',
  line2: 'Ready',
  is_playing: true,
  volume: 40
};

function log(...args) {
  if (['debug', 'info'].includes(LOG_LEVEL)) {
    console.log(new Date().toISOString(), ...args);
  }
}

app.use(express.json());

app.get('/zones', (_req, res) => res.json(zones));

app.get('/now_playing', (req, res) => {
  const zoneId = req.query.zone_id || zones[0].zone_id;
  log('now_playing', zoneId);
  res.json({ ...nowPlayingStore, zone_id: zoneId });
});

app.post('/control', (req, res) => {
  const { action, value } = req.body || {};
  log('control', action, value);
  if (action === 'play_pause') {
    nowPlayingStore.is_playing = !nowPlayingStore.is_playing;
  } else if (action === 'vol_rel' && typeof value === 'number') {
    nowPlayingStore.volume = Math.max(0, Math.min(100, nowPlayingStore.volume + value));
  } else if (action === 'vol_abs' && typeof value === 'number') {
    nowPlayingStore.volume = Math.max(0, Math.min(100, value));
  }
  res.json({ status: 'ok', state: nowPlayingStore });
});

app.get('/status', (_req, res) => {
  res.json({ status: 'ok', version: '0.1.0' });
});

app.get('/image', (_req, res) => {
  res.status(204).end();
});

app.listen(PORT, () => {
  log(`roon-knob bridge listening ${PORT}`);
  advertise(PORT, { base: BRIDGE_BASE });
});
