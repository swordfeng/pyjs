// Copyright (c) 2010, Chris Dickinson
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// Neither the name of the Node-Python binding nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Origin from https://github.com/JeanSebTr/node-python/blob/master/test/index.js

var python = require('../');
require('chai').should();
var expect = require('chai').expect;

python.implicitConversion = false;
python.import('sys').path.insert(python.builtins.int(0), __dirname + '/../');
python.implicitConversion = true;

describe('node-python', function () {
  describe('eval', function () {
    it('should return resulting value from python statement executed', function () {
      var value = python.eval('"1"');
      value.should.equal("1");
    });
    it('should return resulting value from python statement executed, converting to string with complex types', function () {
      var decimal = python.import('decimal');
      var smallNum = decimal.Decimal('0.0000000001');
      smallNum.toString().should.equal('1E-10');
    });
  });
  describe('import', function () {
    it('should return object representing module imported, containing functions from imported module', function () {
      var value = python.import('decimal');
      value.should.have.property('valueOf');
    });
    it('should throw a PythonError when importing a module that does not exist', function () {
      expect(function () {
        python.import('jibberish');
    }).throw(/No module named \'jibberish\'/);
    });
    it('should throw an Error when importing a module that includes bad syntax', function () {
    expect(function () {
        python.import('test.support.test');
    }).throw(/SyntaxError: invalid syntax/)
    });
  });
  it('should convert javascript null to python NoneType', function () {
    test = python.import('test.support.test2');
    var type = test.getPythonTypeName(null);
    type.should.equal('NoneType');
  });
  it('should convert javascript undefined to python NoneType', function () {
    test = python.import('test.support.test2');
    var type = test.getPythonTypeName(undefined);
    type.should.equal('NoneType');
  });
  it('should convert javascript booleans to python booleans', function () {
    test = python.import('test.support.test2');
    var type = test.getPythonTypeName(true);
    type.should.equal('bool');
  });
  it('should convert javascript date to python date', function () {
    test = python.import('test.support.test2');
    var type = test.getPythonTypeName(new Date());
    type.should.equal('datetime');
  });
  it('should convert javascript numbers to python floats', function () {
    test = python.import('test.support.test2');
    var type = test.getPythonTypeName(1);
    type.should.equal('float');
  });
  it('should convert javascript arrays to python list', function () {
    test = python.import('test.support.test2');
    var type = test.getPythonTypeName([]);
    type.should.equal('list');
  });
  it('should convert javascript objects to python dictionaries', function () {
    test = python.import('test.support.test2');
    var type = test.getPythonTypeName({});
    type.should.equal('dict');
  });
  it('should convert javascript nested objects correctly', function () {
    test = python.import('test.support.test2');
    var type = test.getPythonTypeName2({
      value: 1
    }, 'value');
    type.should.equal('float');
    var type = test.getPythonTypeName2({
      value: true
    }, 'value');
    type.should.equal('bool');
    var type = test.getPythonTypeName2({
      value: new Date()
    }, 'value');
    type.should.equal('datetime');
    var type = test.getPythonTypeName2({
      value: {}
    }, 'value');
    type.should.equal('dict');
    var type = test.getPythonTypeName2({
      value: ['one', 'two', 'three']
    }, 'value');
    type.should.equal('list');
    var i = 0, arr = [];
    while (i < 10000) {
      arr.push(Math.random().toString())
      i++;
    }
    var type = test.getPythonTypeName(arr);
    type.should.equal('list');
  });
  it('should convert python dicts to javascript objects', function () {
    test = python.import('test.support.test2');
    var value = test.getPythonValue({
      value: 1
    });
    value.should.have.property('value', 1);
  });
  it('should convert python lists to javascript arrays', function () {
    test = python.import('test.support.test2');
    var value = test.getPythonValue([ 1, 2, 3]);
    value.should.contain(1);
    value.should.contain(2);
    value.should.contain(3);
  });
});
