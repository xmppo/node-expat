'use strict'

const util = require('util')
const expat = require('bindings')('node_expat')
const Stream = require('stream').Stream

const Parser = function (encoding) {
  this.encoding = encoding
  this._getNewParser()
  this.parser.emit = this.emit.bind(this)

  // Stream API
  this.writable = true
  this.readable = true
}
util.inherits(Parser, Stream)

Parser.prototype._getNewParser = function () {
  this.parser = new expat.Parser(this.encoding)
}

Parser.prototype.parse = function (buf, isFinal) {
  return this.parser.parse(buf, isFinal)
}

Parser.prototype.setEncoding = function (encoding) {
  this.encoding = encoding
  return this.parser.setEncoding(this.encoding)
}

Parser.prototype.setUnknownEncoding = function (map, convert) {
  return this.parser.setUnknownEncoding(map, convert)
}

Parser.prototype.getError = function () {
  return this.parser.getError()
}
Parser.prototype.stop = function () {
  return this.parser.stop()
}
Parser.prototype.pause = function () {
  return this.stop()
}
Parser.prototype.resume = function () {
  return this.parser.resume()
}

Parser.prototype.destroy = function () {
  this.parser.stop()
  this.end()
  return this
}

Parser.prototype.destroySoon = function () {
  this.destroy()
}

Parser.prototype.write = function (data) {
  let error, result
  try {
    result = this.parse(data)
    if (!result) {
      error = this.getError()
    }
  } catch (e) {
    error = e
  }
  if (error) {
    this.emit('error', error)
    this.emit('close')
  }
  return result
}

Parser.prototype.end = function (data) {
  let error, result
  try {
    result = this.parse(data || '', true)
    if (!result) {
      error = this.getError()
    }
  } catch (e) {
    error = e
  }

  if (!error) {
    this.emit('end')
  } else {
    this.emit('error', error)
  }
  this.emit('close')
}

Parser.prototype.reset = function () {
  return this.parser.reset()
}
Parser.prototype.getCurrentLineNumber = function () {
  return this.parser.getCurrentLineNumber()
}
Parser.prototype.getCurrentColumnNumber = function () {
  return this.parser.getCurrentColumnNumber()
}
Parser.prototype.getCurrentByteIndex = function () {
  return this.parser.getCurrentByteIndex()
}

exports.Parser = Parser

exports.createParser = function (cb) {
  const parser = new Parser()
  if (cb) {
    parser.on('startElement', cb)
  }
  return parser
}
