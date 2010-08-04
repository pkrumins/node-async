var Async = require('../async').Async;

var async = new Async(11,4);

async.moo(function (x) {
 console.log(x);
});

