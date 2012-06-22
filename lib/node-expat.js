var EventEmitter = require('events').EventEmitter;
var util = require('util');
var Stream = require('stream').Stream;

try {
  var expat = require('../build/Release/node-expat');
} catch(e) {
  var expat = require('../build/default/node-expat');
}

var Parser = function(encoding) {
  var that = this;

  this.parser = new expat.Parser(encoding);
  this.parser.emit = function() {
    that.emit.apply(that, arguments);
  };

  //stream API
  this.writable = true;
  this.readable = true;
};
util.inherits(Parser, Stream);

Parser.prototype.parse = function(buf, isFinal) {
  this.emit('data', buf);
  return this.parser.parse(buf, isFinal);
};

Parser.prototype.setEncoding = function(encoding) {
  return this.parser.setEncoding(encoding);
};

Parser.prototype.getError = function() {
  return this.parser.getError();
};
Parser.prototype.stop = function() {
  return this.parser.stop();
};
Parser.prototype.pause = function() {
  return this.stop();
};
Parser.prototype.resume = function() {
  return this.parser.resume();
};

Parser.prototype.destroy = function() {
  this.parser.stop();
  this.end();
};

Parser.prototype.destroySoon = function() {
  this.destroy();
};

Parser.prototype.write = Parser.prototype.parse;

Parser.prototype.end = function() {
  this.emit('close');
  this.emit('end');
};


//Exports

exports.Parser = Parser;

exports.createParser = function(cb) {
  var parser = new Parser();
  if(cb)  { parser.on('startElement', cb); }
  return parser;
};
exports.Parser.prototype.reset = function() {
    return this.parser.reset();
};
exports.Parser.prototype.getCurrentLineNumber = function() {
    return this.parser.getCurrentLineNumber();
};
exports.Parser.prototype.getCurrentColumnNumber = function() {
    return this.parser.getCurrentColumnNumber();
};
exports.Parser.prototype.getCurrentByteIndex = function() {
    return this.parser.getCurrentByteIndex();
};
