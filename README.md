# node-expat

[![build status](https://img.shields.io/travis/node-xmpp/node-expat/master.svg?style=flat-square)](https://travis-ci.org/node-xmpp/node-expat/branches)
[![js-standard-style](https://img.shields.io/badge/code%20style-standard-brightgreen.svg?style=flat-square)](http://standardjs.com/)

## Motivation

You use [Node.js](https://nodejs.org) for speed? You process
XML streams? Then you want the fastest XML parser: [libexpat](http://expat.sourceforge.net/)!

## Manual

Please see the [node-expat manual](http://node-xmpp.org/doc/expat.html)

## Install

```
npm install node-expat
```

## Testing

```
npm install -g standard
npm test
```

## Benchmark

```node benchmark.js```

| module             | ops/sec | native | XML compliant |
|--------------------|--------:|:------:|:-------------:|
| sax                |  18,641 | ☐      | ☑             |
| node-xml           |  49,257 | ☐      | ☑             |
| libxmljs           |  95,169 | ☑      | ☑             |
| **node-expat**     | 130,776 | ☑      | ☑             |
| ltx/lib/parsers/ltx| 172,596 | ☐      | ☐             |

Higher is better. Please note that ltx parser is not entirely XML compliant.
