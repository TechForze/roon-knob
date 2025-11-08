const express = require('express');

function createRoutes({ bridge }) {
  const router = express.Router();

  router.get('/zones', (_req, res) => {
    res.json(bridge.getZones());
  });

  router.get('/now_playing', (req, res) => {
    const zoneId = req.query.zone_id;
    const data = bridge.getNowPlaying(zoneId);
    if (!data) {
      return res.status(404).json({ error: 'zone not found or no data yet' });
    }
    res.json(data);
  });

  router.post('/control', async (req, res) => {
    const { zone_id, action, value } = req.body || {};
    if (!zone_id || !action) {
      return res.status(400).json({ error: 'zone_id and action required' });
    }
    try {
      await bridge.control(zone_id, action, value);
      res.json({ status: 'ok' });
    } catch (error) {
      res.status(500).json({ error: error.message || 'control failed' });
    }
  });

  router.get('/now_playing/mock', (_req, res) => {
    res.json({
      line1: 'Mock Track',
      line2: 'Mock Artist',
      is_playing: true,
      volume: 30,
      volume_step: 2,
    });
  });

  router.get('/image', (_req, res) => {
    res.status(204).end();
  });

  return router;
}

module.exports = { createRoutes };
