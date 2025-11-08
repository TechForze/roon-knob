const express = require('express');
const morgan = require('morgan');
const cors = require('cors');
const path = require('path');
const { advertise } = require('./mdns');
const { createRoutes } = require('./routes');
const { createRoonBridge } = require('./bridge');
const { createMetricsTracker } = require('./metrics');
const { createLogger } = require('./logger');

function startServer() {
  const PORT = parseInt(process.env.PORT || '8088', 10);
  const MDNS_NAME = process.env.MDNS_NAME || 'Roon Knob Bridge';
  const LOG_LEVEL = process.env.LOG_LEVEL || 'info';
  const SERVICE_PORT = parseInt(process.env.ROON_SERVICE_PORT || '9330', 10);

  const metrics = createMetricsTracker();
  const log = createLogger('Sidecar');
  const app = express();
  app.use(express.json());
  app.use(cors());
  app.use(morgan(LOG_LEVEL === 'debug' ? 'dev' : 'tiny'));

  const bridge = createRoonBridge({ service_port: SERVICE_PORT, display_name: MDNS_NAME, log: createLogger('Bridge') });
  bridge.start();

  app.use(createRoutes({ bridge, metrics, log: createLogger('HTTP') }));

  app.get('/status', (_req, res) => {
    res.json({ status: 'ok', version: '0.1.0' });
  });

  app.use(express.static(path.join(__dirname, 'public')));

  app.get('/', (_req, res) => {
    res.redirect('/admin');
  });

  app.use((err, req, res, _next) => {
    log.error('Unhandled request error', { path: req.path, err });
    res.status(500).json({ error: 'internal_error' });
  });

  const server = app.listen(PORT, () => {
    log.info(`Listening on ${PORT}`, { service_port: SERVICE_PORT });
    try {
      advertise(PORT, {
        name: MDNS_NAME,
        base: `http://${require('os').hostname()}:${PORT}`,
        txt: { api: '1' },
      });
    } catch (error) {
      log.error('mDNS advertise failed', { error });
    }
    metrics.mdns = {
      name: MDNS_NAME,
      port: PORT,
      base: `http://${require('os').hostname()}:${PORT}`,
      advertisedAt: Date.now(),
    };
  });

  server.on('error', (err) => {
    if (err.code === 'EADDRINUSE') {
      log.error('Port already in use. Stop the other process or set PORT.', { port: PORT });
    } else {
      log.error('HTTP server error', { err });
    }
    process.exit(1);
  });

  process.on('unhandledRejection', (err) => {
    log.error('Unhandled promise rejection', { err });
  });

  process.on('uncaughtException', (err) => {
    log.error('Uncaught exception', { err });
  });

  return { server, bridge };
}

module.exports = { startServer };
