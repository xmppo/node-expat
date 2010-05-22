var sys = require('sys');
var expat = require('./build/default/expat');

function collapseTexts(evs) {
    var r = [];
    var t = "";
    evs.forEach(function(ev) {
	if (ev[0] == 'text')
	    t += ev[1];
	else {
	    if (t != "")
		r.push(['text', t]);
	    t = "";
	    r.push(ev);
	}
    });
    if (t != "")
	r.push(['text', t]);
    return r;
}

function expect(s, evs_expected) {
    for(var step = s.length; step > 0; step--) {
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
	for(var l = 0; l < s.length; l += step)
	    p.parse(s.substr(l, step), false);

	var expected = JSON.stringify(evs_expected);
	var received = JSON.stringify(collapseTexts(evs_received));
	if (expected != received) {
	    sys.puts("Fail for: " + s + " (step=" + step + ")");
	    sys.puts("Expected: " + expected);
	    sys.puts("Received: " + received);
	}
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
expect("<r>foo</r>",
       [['startElement', 'r', {}],
	['text', "foo"],
	['endElement', 'r']]);
expect("<r>foo\nbar</r>",
       [['startElement', 'r', {}],
	['text', "foo\nbar"],
	['endElement', 'r']]);
expect("<r><![CDATA[<greeting>Hello, world!</greeting>]]></r>",
       [['startElement', 'r', {}],
	['text', "<greeting>Hello, world!</greeting>"],
	['endElement', 'r']]);
expect("<r>foo&amp;bar</r>",
       [['startElement', 'r', {}],
	['text', "foo&bar"],
	['endElement', 'r']]);
expect("<r>ß</r>",
       [['startElement', 'r', {}],
	['text', "ß"],
	['endElement', 'r']]);
