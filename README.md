# node-expat

[![build status](https://img.shields.io/travis/astro/node-expat/master.svg?style=flat-square)](https://travis-ci.org/astro/node-expat/branches)
[![js-standard-style](https://img.shields.io/badge/code%20style-standard-brightgreen.svg?style=flat-square)](http://standardjs.com/)

## Motivation

You use [Node.js](https://nodejs.org) for speed? You process XML streams? Then you want the fastest XML parser: [libexpat](http://expat.sourceforge.net/)!

## Install

```
npm install node-expat
```

## Usage

Important events emitted by a parser:

```javascript
(function () {
  "use strict";

  var expat = require('node-expat')
  var parser = new expat.Parser('UTF-8')

  parser.on('startElement', function (name, attrs) {
    console.log(name, attrs)
  })

  parser.on('endElement', function (name) {
    console.log(name)
  })

  parser.on('text', function (text) {
    console.log(text)
  })

  parser.on('error', function (error) {
    console.error(error)
  })

  parser.write('<html><head><title>Hello World</title></head><body><p>Foobar</p></body></html>')

}())

```

## API

* `#on('startElement' function (name, attrs) {})`
* `#on('endElement' function (name) {})`
* `#on('text' function (text) {})`
* `#on('processingInstruction', function (target, data) {})`
* `#on('comment', function (s) {})`
* `#on('xmlDecl', function (version, encoding, standalone) {})`
* `#on('startCdata', function () {})`
* `#on('endCdata', function () {})`
* `#on('entityDecl', function (entityName, isParameterEntity, value, base, systemId, publicId, notationName) {})`
* `#on('error', function (e) {})`
* `#stop()` pauses
* `#resume()` resumes

## Error handling

We don't emit an error event because libexpat doesn't use a callback
either. Instead, check that `parse()` returns `true`. A descriptive
string can be obtained via `getError()` to provide user feedback.

Alternatively, use the Parser like a node Stream. `write()` will emit
error events.

## Namespace handling

A word about special parsing of *xmlns:* this is not necessary in a
bare SAX parser like this, given that the DOM replacement you are
using (if any) is not relevant to the parser.

## Benchmark

`npm run benchmark`

| module                                                                                | ops/sec | native | XML compliant | stream         |
|---------------------------------------------------------------------------------------|--------:|:------:|:-------------:|:--------------:|
| [sax-js](https://github.com/isaacs/sax-js)                                            |  99,412 | ☐      | ☑             | ☑              |
| [node-xml](https://github.com/dylang/node-xml)                                        | 130,631 | ☐      | ☑             | ☑              |
| [libxmljs](https://github.com/polotek/libxmljs)                                       | 276,136 | ☑      | ☑             | ☐              |
| **node-expat**                                                                        | 322,769 | ☑      | ☑             | ☑              |

Higher is better.

## Testing

```
npm install -g standard
npm test
```

## Windows

If you fail to install node-expat as a dependency of node-xmpp, please update node-xmpp as it doesn't use node-expat anymore.

Dependencies for `node-gyp` https://github.com/TooTallNate/node-gyp#installation

See https://github.com/astro/node-expat/issues/78 if you are getting errors about not finding `nan.h`.

### expat.vcproj

```
VCBUILD : error : project file 'node-expat\build\deps\libexpat\expat.vcproj' was not found or not a valid proj
ect file. [C:\Users\admin\AppData\Roaming\npm\node_modules\node-expat\build\bin
ding.sln]
```

Install [Visual Studio C++ 2012](http://go.microsoft.com/?linkid=9816758) and run npm with the [`--msvs_version=2012` flag](http://stackoverflow.com/a/16854333/937891).
