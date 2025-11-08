function timestamp() {
  return new Date().toISOString();
}

function log(scope, level, message, meta) {
  const parts = [`[${timestamp()}][${scope}][${level.toUpperCase()}]`, message];
  if (meta && Object.keys(meta).length) {
    parts.push(JSON.stringify(meta));
  }
  console.log(parts.join(' '));
}

function createLogger(scope) {
  return {
    info: (msg, meta = {}) => log(scope, 'info', msg, meta),
    warn: (msg, meta = {}) => log(scope, 'warn', msg, meta),
    error: (msg, meta = {}) => log(scope, 'error', msg, meta),
    debug: (msg, meta = {}) => {
      if ((process.env.LOG_LEVEL || '').toLowerCase() === 'debug') {
        log(scope, 'debug', msg, meta);
      }
    },
  };
}

module.exports = { createLogger };
