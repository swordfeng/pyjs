require('./common');
describe('import', function () {
    it('import single value', function () {
        var test = py.import('test');
        assert.equal(5.0, test.$attr('a'));
    });
    it('set value', function () {
        var test = py.import('test');
        test.$attr('b', 3);
        assert.equal(3, test.$attr('b'));
    });
});
