'use strict'

const expat = require('../lib/node-expat')
const vows = require('vows')
const assert = require('assert')

function throwingMapAt (index) {
  const m = []
  for (let i = 0; i < 256; i++) {
    m[i] = i
  }
  Object.defineProperty(m, index, {
    get: function () { throw new Error('element getter') },
    enumerable: true,
    configurable: true
  })
  return m
}

function setUnknownEncoding (map) {
  let inner = null
  const p = new expat.Parser()
  p.on('unknownEncoding', function () {
    try {
      p.parser.setUnknownEncoding(map)
    } catch (err) {
      inner = err
    }
  })
  p.parse('<?xml version="1.0" encoding="x-weird"?><root/>')
  return inner
}

function parseWithThrowingEmit (xml) {
  const p = new expat.Parser()
  Object.defineProperty(p.parser, 'emit', {
    get: function () { throw new Error('emit getter') },
    configurable: true
  })
  return function () { p.parse(xml) }
}

vows.describe('argument types').addBatch({
  setUnknownEncoding: {
    'propagates a throwing getter on the first element': function () {
      assert.ok(setUnknownEncoding(throwingMapAt(0)) instanceof Error)
    },
    'propagates a throwing getter on a middle element': function () {
      assert.ok(setUnknownEncoding(throwingMapAt(5)) instanceof Error)
    },
    'propagates a throwing getter on the last element': function () {
      assert.ok(setUnknownEncoding(throwingMapAt(255)) instanceof Error)
    }
  },
  emit: {
    'propagates a throwing emit getter on an element event': function () {
      assert.throws(parseWithThrowingEmit('<r>hi</r>'))
    },
    'propagates a throwing emit getter on a comment event': function () {
      assert.throws(parseWithThrowingEmit('<!-- c --><r/>'))
    }
  }
}).export(module)
