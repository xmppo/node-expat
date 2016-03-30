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

`npm run benchmark`

| module                                                                                | ops/sec | native | XML compliant | stream         |
|---------------------------------------------------------------------------------------|--------:|:------:|:-------------:|:--------------:|
| [sax-js](https://github.com/isaacs/sax-js)                                            |  65,853 | ☐      | ☑             | ☑              |
| [node-xml](https://github.com/dylang/node-xml)                                        | 106,374 | ☐      | ☑             | ☑              |
| [libxmljs](https://github.com/polotek/libxmljs)                                       | 194,386 | ☑      | ☑             | ☐              |
| **node-expat**                                                                        | 222,707 | ☑      | ☑             | ☑              |
| [ltx/lib/parsers/ltx](https://github.com/node-xmpp/ltx/blob/master/lib/parsers/ltx.js)| 473,628 | ☐      | ☐             | ☑              |

Higher is better.


## Windows

If you fail to install node-expat as a dependency of node-xmpp, please update node-xmpp as it doesn't use node-expat anymore. Help is welcome as none of the maintainers are currently interested in Windows support. See https://github.com/node-xmpp/node-expat/issues/132 .
