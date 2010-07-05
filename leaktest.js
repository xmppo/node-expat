var sys = require('sys');
var expat = require('./build/default/expat');

var i = 0;

function foo() {
    var p = new expat.Parser();
    process.nextTick(foo);
    i++;
}

foo();

setInterval(function() {
		var mem = process.memoryUsage();
		sys.print(i+" Iterations: ");
		for(var k in mem) {
		    sys.print("\t"+k+"="+mem[k]);
		}
		sys.print("\n");

		i = 0;
	    }, 100);
