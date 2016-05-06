require('./common');

describe('error handling', function () {
    it('throw from python code', function () {
        (() => testModule.willraise()).should.throw(/test/);
    });
});
