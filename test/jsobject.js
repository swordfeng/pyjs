require('./common');

describe('PyObject', function () {
    describe('type conversion', function () {
        it('null <---> None', function () {
            assert.equal('None', new PyObject(null).$repr());
            assert.equal(null, new PyObject(null).$value());
        });
        it('true <---> True', function () {
            assert.equal('True', new PyObject(true).$repr());
            assert.equal(true, new PyObject(true).$value());
        });
        it('false <---> False', function () {
            assert.equal('False', new PyObject(false).$repr());
            assert.equal(false, new PyObject(false).$value());
        });
        it('undefined <---> (nullptr)', function () {
            assert.equal(undefined, new PyObject().$value());
        });
        it('string <---> unicode', function () {
            var crypto = require('crypto');
            var str = crypto.randomBytes(1024).toString('utf8');
            assert.equal(str, new PyObject(str).$value());
        });
        it('number <---> double', function () {
            assert.equal(15, new PyObject(15).$value());
            assert.equal('15.0', new PyObject(15).$repr());
        });
        it('array <---> list', function () {
            var arr = new PyObject([1, 'aa', false]).$value();
            assert.deepEqual([1, 'aa', false], arr);
        });
        it('normal object <---> dict', function () {
            var obj = {
                'a': 1,
                2: 'str',
                'x y': true,
                'something': null
            };
            assert.deepEqual(obj, new PyObject(obj).$value());
        });
        it('buffer <---> bytes', function () {
            var bytes = new PyObject(Buffer('abcd', 'utf8'));
            assert.equal('b\'abcd\'', bytes.toString());
            var newBuf = bytes.$value();
            assert(newBuf instanceof Buffer);
            assert.equal('abcd', newBuf.toString('utf8'));
        });
        it('self wrap', function () {
            var pyobj = new PyObject(15);
            pyobj = new PyObject(15);
            assert.equal(15, pyobj.$value());
        });
    });
    describe('py function conversion', function () {
        it('function', function() {
            var test = py.import('test');
            var func = test.$attr('func');
            assert.equal('function', typeof func);
            assert.equal(true, func.__proto__ instanceof PyObject);
            assert.equal('hello', func());
        });
        it('function2', function() {
            var test = py.import('test');
            var func2 = test.$attr('func2');
            assert.equal(5, func2.$call([2]));
            assert.equal(5, func2(2));
        });
    });
    describe('prototype', function () {
        it('a.__proto__.__proto__ === PyObject.prototype', function () {
            var a = PyObject('abc');
            function makeb() {}
            makeb.prototype = a;
            assert(new makeb().$value(), 'abc');
        });
    });
    describe('getter and setters', function () {
        it('getter', function () {
            var a = builtins.int(15);
            assert.equal(a.bit_length(), 4);
        });
        it('setter', function () {
            var testclass = py.import('test').testclass;
            var a = testclass();
            assert.equal(a.a, 2);
            a.a = 4;
            assert.equal(a.a, 4);
            assert.deepEqual(a.$('__dict__'), { a: 4 });
        });
    });
    describe('valueOf', function () {
        it ('int', function () {
            var a = builtins.int(32);
            var b = builtins.float(10);
            assert.equal(a + b, 42);
        });
    });
    describe('ref count?', function () {
        it('repeat wrap', function () {
            var a = 'teststring';
            var b = 'teststring';
            for (var i = 0; i < 100; i++) {
                a = PyObject(a);
                assert.equal(a.$value(), b);
            }
        });
        it('concat string', function () {
            var a = 'teststring';
            var b = 'teststring';
            for (var i = 0; i < 100; i++) {
                a = PyObject(a).$('__add__')('b');
                b += 'b';
                assert.equal(a, b);
            }
        });
        it('no leak', function () {
            var a = 'teststring';
            var b = 'teststring';
            for (var i = 0; i < 100000; i++) {
                PyObject(a);
            }
        });
    });
});
