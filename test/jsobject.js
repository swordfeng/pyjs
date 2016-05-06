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
        it('undefined <---> None (just for now)', function () {
            assert.equal(PyObject().$value(), null);
        });
        it('number <---> double', function () {
            assert.equal(15, new PyObject(15).$value());
            assert.equal('15.0', new PyObject(15).$repr());
        });
        it('string <---> unicode', function () {
            var crypto = require('crypto');
            var str = crypto.randomBytes(1024).toString('utf8');
            assert.equal(str, new PyObject(str).$value());
        });
        it('array <---> list', function () {
            var arr = new PyObject([1, 'aa', false]).$value();
            assert.deepEqual([1, 'aa', false], arr);
        });
        it('buffer <---> bytes', function () {
            var bytes = new PyObject(Buffer('abcd', 'utf8'));
            assert.equal('b\'abcd\'', bytes.toString());
            var newBuf = bytes.$value();
            assert(newBuf instanceof Buffer);
            assert.equal('abcd', newBuf.toString('utf8'));
        });
        it('Object <---> dict', function () {
            var obj = {
                'a': 1,
                2: 'str',
                'x y': true,
                'something': null
            };
            assert.deepEqual(obj, new PyObject(obj).$value());
        });
        it('PyObject <---> object', function () {
            var pyobj = new PyObject(15);
            pyobj.should.be.instanceof(PyObject);
            assert.equal(15, PyObject(pyobj).$value());
        });
    });
    describe('function conversion', function () {
        it('should convert python function to javascript function', function() {
            (typeof testModule.function1).should.equal('function');
            testModule.function1.should.be.an.instanceof(PyObject);
            testModule.function1().should.equal('hello');
            testModule.function2(2).should.equal(5);
            testModule.function2.$call([3]).should.equal(7);
        });
        it('should convert javascript function to python function', function() {
            var pyfunc = PyObject(function (x) { return x + 1; })
            pyfunc.should.be.instanceof(PyObject);
            pyfunc.$value().should.be.a('function');
            testModule.callFunction(pyfunc, 1).should.equal(2);
            testModule.callFunction(pyfunc, 'a').should.equal('a1');
        });
    });
    describe('prototype', function () {
        it('should find the correct wrapper in prototype chain', function () {
            var a = PyObject('abc');
            var b = { __proto__: a };
            var c = { __proto__: b };
            b.$value().should.equal('abc');
            c.$value().should.equal('abc');
        });
        it('should throw if nothing is on prototype chain', function () {
            var a = PyObject('abc');
            var b = { $value: a.$value };
            (() => b.$value()).should.throw(/Unexpected object/);
        });
    });
    describe('getter and setters', function () {
        it('getter', function () {
            var a = builtins.int(15);
            assert.equal(a.bit_length(), 4);
        });
        it('setter', function () {
            var a = testModule.testclass();
            assert.equal(a.a, 2);
            a.a = 4;
            assert.equal(a.a, 4);
            assert.deepEqual(a.$('__dict__'), { a: 4 });
        });
    });
    describe('valueOf', function () {
        it ('should convert int to number', function () {
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
