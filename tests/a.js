var Async = require('../async').Async;

var async = new Async(1,2);

async.moo(function (x) {
 console.log(x);
});

