require('./common');
describe('builtins', function () {
    it('len', function () {
        assert.equal(2, builtins.len([1,2]));
    });
    it('large integer add', function () {
        var inta = builtins.int('340282366920938463463374607431768211455');
        var intb = builtins.int(2);
        var intc = inta.$('__add__')(intb);
        assert.equal('340282366920938463463374607431768211457', intc.toString());
    });
});
