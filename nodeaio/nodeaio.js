const py = require('..')
const nodeaio = py.import('nodeaio.nodeaio')

function aioco(coro) {
    return new Promise(nodeaio.ensure_coroutine(coro).then);
}

process.on('beforeExit', () => nodeaio.loopt.stop());

module.exports = {
    aioco
};
