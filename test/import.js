'use strict';
var pyjs = require('../build/Release/pyjs-native.node');
var assert = require('chai').assert;
var pyimport = pyjs.import;

describe('import', function () {
    it('import single value', function () {
        var test = pyimport('test');
        assert.equal(5.0, test.attr('a'));
    });
    it('set value', function () {
        var test = pyimport('test');
        test.attr('b', 3);
        assert.equal(3, test.attr('b'));
    });
});
