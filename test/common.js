'use strict';
global.py = require('../');
global.assert = require('chai').assert;
global.expect = require('chai').expect;
require('chai').should();

global.PyObject = py.PyObject;
global.builtins = py.builtins;

before(function () {
    var sys = py.import('sys');
    if (sys.path.indexOf('') === -1) {
        sys.path = [''].concat(sys.path);

        global.testModule = py.import('test');
    }
});
