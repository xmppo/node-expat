var sys = require('sys');
var expat = require('./build/default/expat');

sys.puts("newing");
var p = new expat.Parser();
sys.puts("newed");
p.parse("<foo bar='baz'/>");
