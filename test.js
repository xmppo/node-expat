var sys = require('sys');
var expat = require('./build/default/node-expat');
var Buffer = require('buffer').Buffer;

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

var tests = 0, iterations = 0, fails = 0;
function expect(s, evs_expected) {
    tests++;
    for(var step = s.length; step > 0; step--) {
	iterations++;
	var evs_received = [];
	var p = new expat.Parser("UTF-8");
	//p.setEncoding("UTF-8");
	p.addListener('startElement', function(name, attrs) {
	    evs_received.push(['startElement', name, attrs]);
	});
	p.addListener('endElement', function(name) {
	    evs_received.push(['endElement', name]);
	});
	p.addListener('text', function(s) {
	    evs_received.push(['text', s]);
	});
	p.addListener('processingInstruction', function(target, data) {
	    evs_received.push(['processingInstruction', target, data]);
	});
	p.addListener('comment', function(s) {
	    evs_received.push(['comment', s]);
	});
	p.addListener('xmlDecl', function(version, encoding, standalone) {
	    evs_received.push(['xmlDecl', version, encoding, standalone]);
	});
	p.addListener('startCdata', function() {
	    evs_received.push(['startCdata']);
	});
	p.addListener('endCdata', function() {
	    evs_received.push(['endCdata']);
	});
	p.addListener('entityDecl', function(entityName, isParameterEntity, value, base, systemId, publicId, notationName) {
	    evs_received.push(['entityDecl', entityName, isParameterEntity, value, base, systemId, publicId, notationName]);
	});
p.on('drain',function(){console.log('drain');});
	for(var l = 0; l < s.length; l += step)
	{
	    var end = l + step;
	    if (end > s.length)
		end = s.length;

	    if (!p.parse(s.slice(l, end), false))
		evs_received.push(['error']);
	}

	var expected = JSON.stringify(evs_expected);
	var received = JSON.stringify(collapseTexts(evs_received));
	if (expected != received) {
	    fails++;
	    sys.puts("Fail for: " + s + " (step=" + step + ")");
	    sys.puts("Expected: " + expected);
	    sys.puts("Received: " + received);
	    return;  // don't try with smaller step size
	}
    }
}

function testStopResume(cb)
{
	var p = new expat.Parser("UTF-8");
	
	var input = '\
		<wrap> \
			<short /> \
			<short></short> \
			<long /> \
			<short /> \
			<long>foo</long> \
		</wrap>';
	
	var expected = ['wrap', 'short', 'short', 'long', 'short', 'long'];
	var received = [];
	
	var tolerance = 10/100;
	var expectedRuntime = 1000;
	var start = new Date();
	
	p.addListener('startElement', function(name, attrs) {
		received.push(name);
		
		// suspend parser for 1/2 second
		if(name == 'long') {
			p.stop();
			
			setTimeout(function() {
				p.resume();
			}, 500);
		}
	})
	
	p.addListener('endElement', function(name) {
		// finished parsing
		if(name == 'wrap') {
			// test elements received (count. naming, order)
			if(JSON.stringify(expected) != JSON.stringify(received)) {
				sys.puts("Failed Stop/Resume test");
				sys.puts("Expected: " + expected);
				sys.puts("Received: " + received);
				return cb(false);
			}
			
			// test timing (+-5%)
			var now = new Date();
			var diff = now.getTime() - start.getTime();
			var max = expectedRuntime + expectedRuntime * tolerance, 
				min = expectedRuntime - expectedRuntime * tolerance;
			
			if(diff > max) {
				sys.puts("Failed Stop/Resume test");
				sys.puts("Expected Runtime < " + max);
				sys.puts("Taken Runtime: " + diff);
				return cb(false);
			}
			
			if(diff < min) {
				sys.puts("Failed Stop/Resume test");
				sys.puts("Expected Runtime > " + min);
				sys.puts("Taken Runtime: " + diff);
				return cb(false);
			}
			
			return cb(true);
		}
	});
	
	if(!p.parse(input)) {
		sys.puts("Failed Stop/Resume test: parse returned error: "+p.getError());
		return cb(false);
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
	['startCdata'],
	['text', "<greeting>Hello, world!</greeting>"],
	['endCdata'],
	['endElement', 'r']]);
expect("<r>foo&amp;bar</r>",
       [['startElement', 'r', {}],
	['text', "foo&bar"],
	['endElement', 'r']]);
expect("<r>ß</r>",
       [['startElement', 'r', {}],
	['text', "ß"],
	['endElement', 'r']]);
expect("<?i like xml?>",
       [['processingInstruction', 'i', 'like xml']]);
expect("<?dragons?>",
       [['processingInstruction', 'dragons', '']]);
expect("<!-- no comment -->",
       [['comment', ' no comment ']]);
expect("<&", [['error']]);
expect("<?xml version='1.0' encoding='UTF-8'?>",
       [['xmlDecl', '1.0', 'UTF-8', true]]);
expect("<?xml version='1.0'?>",
       [['xmlDecl', '1.0', null, true]]);
expect(new Buffer('<foo>bar</foo>'),
       [['startElement', 'foo', {}],
	['text', 'bar'],
	['endElement', 'foo']]);
expect(new Buffer('<foo><![CDATA[bar]]></foo>'),
       [['startElement', 'foo', {}],
	['startCdata'],
	['text', 'bar'],
	['endCdata'],
	['endElement', 'foo']]);
expect('<!DOCTYPE b [<!ELEMENT b (#PCDATA)>' +
       '<!ENTITY l0 "ha"><!ENTITY l1 "&l0;&l0;"><!ENTITY l2 "&l1;&l1;">' +
       ']><b>&l2;</b>',
       [["entityDecl","l0",false,"ha",null,null,null,null],
	["entityDecl","l1",false,"&l0;&l0;",null,null,null,null],
	["entityDecl","l2",false,"&l1;&l1;",null,null,null,null],
	["startElement","b",{}],["text","hahahaha"],["endElement","b"]]);

sys.puts("Ran "+tests+" tests with "+iterations+" iterations: "+fails+" failures.");

sys.puts("Starting Stop/Resume Test (wait for result...)");
testStopResume(function(success) {
	sys.puts("Stop/Resume Test "+(success ? 'succeeded' : 'failed'));
});

