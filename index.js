'use strict';

if (process.env.NODE_ENV === 'debug') {
    module.exports = require('./build/Debug/pyjs-native.node');
} else {
    module.exports = require('./build/Release/pyjs-native.node');
}

