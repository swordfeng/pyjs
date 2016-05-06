require('./common');

describe('error handling', function () {
    it('throw from python code', function () {
        (() => testModule.error.willraise()).should.throw(/test/);
    });
});
