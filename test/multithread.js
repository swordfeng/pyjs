require('./common');

describe('multithread using Python\'s threading', function () {
    var threading = py.import('threading');
    var time = py.import('time');
    it('should create a thread', function (done) {
        this.timeout(3000);
        var t = threading.Thread.$call([],
            {
                target: testModule.waitThenCall,
                args: builtins.tuple([2, done])
            }
        );
        t.start();
    });
    it('should create a thread and join', function () {
        this.timeout(500);
        var t = threading.Thread.$call([], { target: time.sleep, args: builtins.tuple([0.3])});
        t.start();
        t.join();
    });
});
