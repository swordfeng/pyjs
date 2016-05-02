'use strict';
var pyjs = require('../build/Release/pyjs-native.node');
var assert = require('chai').assert;
var jsesc = require('jsesc');
var PyObject = pyjs.PyObject;

describe('PyObject', function () {
    describe('type conversion', function () {
        it('null <---> None', function () {
            assert.equal('None', new PyObject(null).repr().value());
            assert.equal(null, new PyObject(null).value());
        });
        it('true <---> True', function () {
            assert.equal('True', new PyObject(true).repr().value());
            assert.equal(true, new PyObject(true).value());
        });
        it('false <---> False', function () {
            assert.equal('False', new PyObject(false).repr().value());
            assert.equal(false, new PyObject(false).value());
        });
        it('undefined <---> (nullptr)', function () {
            assert.equal(undefined, new PyObject().value());
        });
        it('string <---> unicode', function () {
            var crypto = require('crypto');
            var str = crypto.randomBytes(1024).toString('utf8');
            assert.equal(str, new PyObject(str).value());
        });
        it('number <---> double', function () {
            assert.equal(15, new PyObject(15).value());
            assert.equal('15.0', new PyObject(15).repr().value());
        });
        it('array <---> list', function () {
            var arr = new PyObject([1, 'aa', false]).value();
            assert.deepEqual([1, 'aa', false], arr);
        });
        it('normal object <---> dict', function () {
            var obj = {
                'a': 1,
                2: 'str',
                'x y': true,
                'something': null
            };
            assert.deepEqual(obj, new PyObject(obj).value());
        });
        it('self wrap', function () {
            var pyobj = new PyObject(15);
            pyobj = new PyObject(15);
            assert.equal(15, pyobj.value());
        })
    });
});
