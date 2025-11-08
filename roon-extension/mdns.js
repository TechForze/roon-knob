const bonjour = require('bonjour')();

function advertise(port, props = {}) {
  const service = bonjour.publish({
    name: props.name || 'Roon Knob Bridge',
    type: 'roonknob',
    protocol: 'tcp',
    port,
    txt: {
      base: props.base || `http://localhost:${port}`,
      api: '1',
      ...props.txt
    }
  });
  process.on('exit', () => service.stop());
  return service;
}

module.exports = { advertise };
