'use strict'

var benchmark = require('benchmark')
var nodeXml = require('node-xml')
var libxml = require('libxmljs')
var expat = require('./')
var sax = require('sax')
var LtxSaxParser = require('ltx/lib/parsers/ltx')

function NodeXmlParser () {
  var parser = new nodeXml.SaxParser(function (cb) {})
  this.parse = function (s) {
    parser.parseString(s)
  }
  this.name = 'node-xml'
}
function LibXmlJsParser () {
  var parser = new libxml.SaxPushParser(function (cb) {})
  this.parse = function (s) {
    parser.push(s, false)
  }
  this.name = 'libxmljs'
}
function SaxParser () {
  var parser = sax.parser()
  this.parse = function (s) {
    parser.write(s).close()
  }
  this.name = 'sax'
}
function ExpatParser () {
  var parser = new expat.Parser()
  this.parse = function (s) {
    parser.parse(s, false)
  }
  this.name = 'node-expat'
}
function LtxParser () {
  var parser = new LtxSaxParser()
  this.parse = function (s) {
    parser.write(s)
  }
  this.name = 'ltx'
}

var parsers = [
  SaxParser,
  NodeXmlParser,
  LibXmlJsParser,
  ExpatParser,
  LtxParser
].map(function (Parser) {
  return new Parser()
})

var suite = new benchmark.Suite('parse')

parsers.forEach(function (parser) {
  parser.parse('<r>')
  suite.add(parser.name, function () {
    parser.parse('<foo bar="baz">quux</foo>')
  })
})

suite.on('cycle', function (event) {
  console.log(event.target.toString())
})
  .on('complete', function () {
    console.log('Fastest is ' + this.filter('fastest').map('name'))
  })
  .run({'async': true})
