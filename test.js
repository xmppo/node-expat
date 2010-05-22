var sys = require('sys');
var expat = require('./build/default/expat');

function expect(s, evs_expected) {
    var evs_received = [];
    var p = new expat.Parser();
    p.addListener('startElement', function(name, attrs) {
	evs_received.push(['startElement', name, attrs]);
    });
    p.addListener('endElement', function(name) {
	evs_received.push(['endElement', name]);
    });
    p.addListener('text', function(s) {
	evs_received.push(['text', s]);
    });
    p.addListener('cdata', function(s) {
	evs_received.push(['cdata', s]);
    });
    p.parse(s, true);

    var expected = JSON.stringify(evs_expected);
    var received = JSON.stringify(evs_received);
    if (expected != received) {
	sys.puts("Fail for: " + s);
	sys.puts("Expected: " + expected);
	sys.puts("Received: " + received);
    }
}

expect("<r/>",
       [['startElement', 'r', {}],
	['endElement', 'r']]);
expect("<r foo='bar'/>",
       [['startElement', 'r', {foo: 'bar'}],
	['endElement', 'r']]);
expect("<r foo='bar' baz=\"quux\" test=\"tset\"/>",
       [['startElement', 'r', {foo: 'bar', baz: 'quux', test: 'tset'}],
	['endElement', 'r']]);
expect("<r xmlns='http://localhost/' xmlns:x=\"http://example.com/\"></r>",
       [['startElement', 'r', {xmlns: 'http://localhost/', 'xmlns:x': 'http://example.com/'}],
	['endElement', 'r']]);
