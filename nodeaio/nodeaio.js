const py = require('..');
const path = require('path');
const sys = py.import('sys');

// trick
py.implicitConversion = false;
sys.path.append(__dirname);
const nodeaio = py.import('nodeaio');
sys.path.remove(__dirname);
py.implicitConversion = true;

function aioco(coro) {
    return new Promise(nodeaio.ensure_coroutine(coro).then);
}

process.on('beforeExit', () => nodeaio.stop());

module.exports = { aioco };
