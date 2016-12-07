const py = require('..')
const nodeaio = py.import('nodeaio.nodeaio')

function ensure_coroutine(coro) {
    return new Promise(nodeaio.ensure_coroutine(coro).then);
}

module.exports = {
    ensure_coroutine,
    start: nodeaio.loopt.start,
    stop: nodeaio.loopt.stop
};
