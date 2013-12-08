# node-expat 

## Motivation 

You use [node.js](http://github.com/ry/node) for speed? You process
XML streams? Then you want the fastest XML parser: [libexpat](http://expat.sourceforge.net/)!

## Speed 

A stupid speed test is supplied in `bench.js`. We measure how many
25-byte elements a SAX parser can process:

- [node-xml](http://github.com/robrighter/node-xml) (pure JavaScript): 23,000 el/s
- [libxmljs](http://github.com/polotek/libxmljs) (libxml2 binding): 77,000 el/s
- [node-expat](http://github.com/astro/node-expat) (libexpat binding, this): 113,000 el/s

These numbers were recorded on a Core 2 2400 MHz.

## Instructions 

Install node-expat:

    npm i node-expat
  
### Installing on windows?

See https://github.com/node-xmpp/node-expat/issues/78 if you are getting errors about not finding `nan.h`.

## Testing

```
npm test
```

[![Build Status](https://travis-ci.org/node-xmpp/node-expat.png?branch=v2.1.2)](https://travis-ci.org/node-xmpp/node-expat)


## Usage ##

Important events emitted by a parser:

```javascript
(function () {
  "use strict";

  var expat = require('node-expat')
    , parser
    ;

  parser = new expat.Parser("UTF-8");
  parser.on('startElement', function (name, attrs) {
    console.log(name, attrs);
  });
  parser.on('endElement', function (name) {
    console.log(name);
  });
  parser.on('text', function (text) {
    console.log(text);
  });
  parser.on('error', function (err) {
    console.error(err); 
  });
  parser.write("<html><head><title>Hello World</title></head><body><p>Foobar</p></body></html>");
}());

```

Use `test.js` for reference.

## API ##

- `#on('startElement' function (name, attrs) {})`
- `#on('endElement' function (name) {})`
- `#on('text' function (text) {})`
- `#on('processingInstruction', function (target, data) {})`
- `#on('comment', function (s) {})`
- `#on('xmlDecl', function (version, encoding, standalone) {})`
- `#on('startCdata', function () {})`
- `#on('startCdata', function () {})`
- `#on('endCdata', function () {})`
- `#on('entityDecl', function (entityName, isParameterEntity, value, base, systemId, publicId, notationName) {})`
- `#on('error', function (e) {})`
- `#stop()` pauses
- `#resume()` resumes

## Error handling ##

We don't emit an error event because libexpat doesn't use a callback
either. Instead, check that `parse()` returns `true`. A descriptive
string can be obtained via `getError()` to provide user feedback.

Alternatively, use the Parser like a node Stream. `write()` will emit
error events.

## Namespace handling ##

A word about special parsing of *xmlns:* this is not neccessary in a
bare SAX parser like this, given that the DOM replacement you are
using (if any) is not relevant to the parser.
