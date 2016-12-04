const py = require('.')
const nodeaio = py.import('nodeaio')

function ensure_coroutine(coro) {
    return new Promise(nodeaio.ensure_coroutine(coro));
}

module.exports = {
    ensure_coroutine,
    start: nodeaio.loopt.start,
    stop: nodeaio.loopt.stop
};
