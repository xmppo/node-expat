var sys = require('sys');
var expat = require('./build/default/expat');

function foo() {
    var p = new expat.Parser();
    process.nextTick(foo);
    console.log(sys.inspect(process.memoryUsage()));
}

foo();
