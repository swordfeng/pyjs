var assert = require('chai').assert;
var builtins = require('../').builtins();

describe('builtins', function () {
    it('len', function () {
        assert(2, builtins.len([1,2]));
    });
});
