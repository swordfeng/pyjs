'use strict';

/*
var pyjs_native = require('./build/Release/pyjs-native.node');
module.exports = function (script) {
    return pyjs_native.eval(script);
}
module.exports.eval = pyjs_native.eval;
*/
module.exports = require('./build/Release/pyjs-native.node');
