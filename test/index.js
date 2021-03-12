'use strict'

const expat = require('../lib/node-expat')
const Iconv = require('iconv').Iconv
const Buffer = require('buffer').Buffer
const vows = require('vows')
const assert = require('assert')
const fs = require('fs')
const path = require('path')
const log = require('debug')('test/index')

function collapseTexts (evs) {
  const r = []
  let t = ''
  evs.forEach(function (ev) {
    if (ev[0] === 'text') {
      t += ev[1]
    } else {
      if (t !== '') {
        r.push(['text', t])
      }
      t = ''
      r.push(ev)
    }
  })
  if (t !== '') {
    r.push(['text', t])
  }
  return r
}

function expect (s, evsExpected) {
  for (let step = s.length; step > 0; step--) {
    expectWithParserAndStep(s, evsExpected, new expat.Parser(), step)
  }
}

function expectWithParserAndStep (s, evsExpected, p, step) {
  const evsReceived = []
  p.addListener('startElement', function (name, attrs) {
    evsReceived.push(['startElement', name, attrs])
  })
  p.addListener('endElement', function (name) {
    evsReceived.push(['endElement', name])
  })
  p.addListener('text', function (s) {
    evsReceived.push(['text', s])
  })
  p.addListener('processingInstruction', function (target, data) {
    evsReceived.push(['processingInstruction', target, data])
  })
  p.addListener('comment', function (s) {
    evsReceived.push(['comment', s])
  })
  p.addListener('xmlDecl', function (version, encoding, standalone) {
    evsReceived.push(['xmlDecl', version, encoding, standalone])
  })
  p.addListener('startCdata', function () {
    evsReceived.push(['startCdata'])
  })
  p.addListener('endCdata', function () {
    evsReceived.push(['endCdata'])
  })
  p.addListener('entityDecl', function (entityName, isParameterEntity, value, base, systemId, publicId, notationName) {
    evsReceived.push(['entityDecl', entityName, isParameterEntity, value, base, systemId, publicId, notationName])
  })
  p.addListener('error', function (e) {
    evsReceived.push(['error', e])
  })
  for (let l = 0; l < s.length; l += step) {
    let end = l + step
    if (end > s.length) {
      end = s.length
    }

    p.write(s.slice(l, end))
  }

  const expected = JSON.stringify(evsExpected)
  const received = JSON.stringify(collapseTexts(evsReceived))
  assert.equal(received, expected)
}

vows.describe('node-expat').addBatch({
  'single element': {
    simple: function () {
      expect('<r/>',
        [['startElement', 'r', {}],
          ['endElement', 'r']])
    },
    'single element with attribute': function () {
      expect("<r foo='bar'/>",
        [['startElement', 'r', { foo: 'bar' }],
          ['endElement', 'r']])
    },
    'single elemeht with differently quoted attributes': function () {
      expect('<r foo=\'bar\' baz="quux" test="tset"/>',
        [['startElement', 'r', { foo: 'bar', baz: 'quux', test: 'tset' }],
          ['endElement', 'r']])
    },
    'single element with namespaces': function () {
      expect('<r xmlns=\'http://localhost/\' xmlns:x="http://example.com/"></r>',
        [['startElement', 'r', { xmlns: 'http://localhost/', 'xmlns:x': 'http://example.com/' }],
          ['endElement', 'r']])
    },
    'single element with text content': function () {
      expect('<r>foo</r>',
        [['startElement', 'r', {}],
          ['text', 'foo'],
          ['endElement', 'r']])
    },
    'single element with text content and line break': function () {
      expect('<r>foo\nbar</r>',
        [['startElement', 'r', {}],
          ['text', 'foo\nbar'],
          ['endElement', 'r']])
    },
    'single element with CDATA content': function () {
      expect('<r><![CDATA[<greeting>Hello, world!</greeting>]]></r>',
        [['startElement', 'r', {}],
          ['startCdata'],
          ['text', '<greeting>Hello, world!</greeting>'],
          ['endCdata'],
          ['endElement', 'r']])
    },
    'single element with entity text': function () {
      expect('<r>foo&amp;bar</r>',
        [['startElement', 'r', {}],
          ['text', 'foo&bar'],
          ['endElement', 'r']])
    },
    'single element with umlaut text': function () {
      expect('<r>ß</r>',
        [['startElement', 'r', {}],
          ['text', 'ß'],
          ['endElement', 'r']])
    },
    'from buffer': function () {
      expect(Buffer.from('<foo>bar</foo>'),
        [['startElement', 'foo', {}],
          ['text', 'bar'],
          ['endElement', 'foo']])
    }
  },
  'entity declaration': {
    'a billion laughs': function () {
      expect('<!DOCTYPE b [<!ELEMENT b (#PCDATA)>' +
      '<!ENTITY l0 "ha"><!ENTITY l1 "&l0;&l0;"><!ENTITY l2 "&l1;&l1;">' +
      ']><b>&l2;</b>',
      [['entityDecl', 'l0', false, 'ha', null, null, null, null],
        ['entityDecl', 'l1', false, '&l0;&l0;', null, null, null, null],
        ['entityDecl', 'l2', false, '&l1;&l1;', null, null, null, null],
        ['startElement', 'b', {}], ['text', 'hahahaha'], ['endElement', 'b']])
    }
  },
  'processing instruction': {
    'with parameters': function () {
      expect('<?i like xml?>',
        [['processingInstruction', 'i', 'like xml']])
    },
    simple: function () {
      expect('<?dragons?>',
        [['processingInstruction', 'dragons', '']])
    },
    'XML declaration with encoding': function () {
      expect("<?xml version='1.0' encoding='UTF-8'?>",
        [['xmlDecl', '1.0', 'UTF-8', true]])
    },
    'XML declaration': function () {
      expect("<?xml version='1.0'?>",
        [['xmlDecl', '1.0', null, true]])
    }
  },
  comment: {
    simple: function () {
      expect('<!-- no comment -->',
        [['comment', ' no comment ']])
    }
  },
  'unknownEncoding with single-byte map': {
    'Windows-1252': function () {
      const p = new expat.Parser()
      let encodingName
      p.addListener('unknownEncoding', function (name) {
        encodingName = name
        const map = []
        for (let i = 0; i < 256; i++) {
          map[i] = i
        }
        map[165] = 0x00A5 // ¥
        map[128] = 0x20AC // €
        map[36] = 0x0024 // $
        p.setUnknownEncoding(map)
      })
      let text = ''
      p.addListener('text', function (s) {
        text += s
      })
      p.addListener('error', function (e) {
        assert.fail(e)
      })
      p.parse("<?xml version='1.0' encoding='Windows-1252'?><r>")
      p.parse(Buffer.from([165, 128, 36]))
      p.parse('</r>')
      assert.equal(encodingName, 'Windows-1252')
      assert.equal(text, '¥€$')
    }
  },
  'unknownEncoding with single-byte map using iconv': {
    'Windows-1252': function () {
      const p = new expat.Parser()
      let encodingName
      p.addListener('unknownEncoding', function (name) {
        encodingName = name
        const iconv = new Iconv(encodingName + '//TRANSLIT//IGNORE', 'UTF-8')
        const map = []

        let d = null
        for (let i = 0; i < 256; i++) {
          try {
            d = iconv.convert(Buffer.from([i])).toString()
          } catch (e) {
            d = '\b'
          }
          map[i] = d.charCodeAt(0)
        }
        p.setUnknownEncoding(map)
      })
      let text = ''
      p.addListener('text', function (s) {
        text += s
      })
      p.addListener('error', function (e) {
        assert.fail(e)
      })
      p.parse("<?xml version='1.0' encoding='Windows-1252'?><r>")
      p.parse(Buffer.from([165, 128, 36]))
      p.parse('</r>')
      assert.equal(encodingName, 'Windows-1252')
      assert.equal('¥€$', text)
    }
  },
  error: {
    'tag name starting with ampersand': function () {
      expect('<&', [['error', 'not well-formed (invalid token)']])
    }
  },

  reset: {
    'complete doc without error': function () {
      const p = new expat.Parser('UTF-8')
      expectWithParserAndStep('<start><first /><second>text</second></start>', [['startElement', 'start', {}], ['startElement', 'first', {}], ['endElement', 'first'], ['startElement', 'second', {}], ['text', 'text'], ['endElement', 'second'], ['endElement', 'start']], p, 1000)
      p.reset()
      expectWithParserAndStep('<restart><third>moretext</third><fourth /></restart>', [['startElement', 'restart', {}], ['startElement', 'third', {}], ['text', 'moretext'], ['endElement', 'third'], ['startElement', 'fourth', {}], ['endElement', 'fourth'], ['endElement', 'restart']], p, 1000)
    },
    'incomplete doc without error': function () {
      const p = new expat.Parser('UTF-8')
      expectWithParserAndStep('<start><first /><second>text</second>', [['startElement', 'start', {}], ['startElement', 'first', {}], ['endElement', 'first'], ['startElement', 'second', {}], ['text', 'text'], ['endElement', 'second']], p, 1000)
      p.reset()
      expectWithParserAndStep('<restart><third>moretext</third><fourth /></restart>', [['startElement', 'restart', {}], ['startElement', 'third', {}], ['text', 'moretext'], ['endElement', 'third'], ['startElement', 'fourth', {}], ['endElement', 'fourth'], ['endElement', 'restart']], p, 1000)
    },
    'with doc error': function () {
      const p = new expat.Parser('UTF-8')
      expectWithParserAndStep('</end>', [['error', 'not well-formed (invalid token)']], p, 1000)
      p.reset()
      expectWithParserAndStep('<restart><third>moretext</third><fourth /></restart>', [['startElement', 'restart', {}], ['startElement', 'third', {}], ['text', 'moretext'], ['endElement', 'third'], ['startElement', 'fourth', {}], ['endElement', 'fourth'], ['endElement', 'restart']], p, 1000)
    }
  },
  'stop and resume': {
    topic: function () {
      const onDone = this.callback
      const p = new expat.Parser('UTF-8')

      const input = [
        '<wrap>',
        '<short />',
        '<short></short>',
        '<long />',
        '<short />',
        '<long>foo</long>',
        '</wrap>'
      ].join('')

      const expected = ['wrap', 'short', 'short', 'long', 'short', 'long']
      const received = []

      const tolerance = 20 / 100
      const expectedRuntime = 1000
      const start = new Date()

      p.addListener('startElement', function (name, attrs) {
        received.push(name)

        // suspend parser for 1/2 second
        if (name === 'long') {
          p.stop()
          setTimeout(function () {
            p.resume()
          }, 500)
        }
      })

      p.addListener('endElement', function (name) {
        // finished parsing
        if (name === 'wrap') {
          // test elements received (count. naming, order)
          assert.equal(JSON.stringify(received), JSON.stringify(expected))

          // test timing (+-20%)
          const now = new Date()
          const diff = now.getTime() - start.getTime()
          const max = expectedRuntime + expectedRuntime * tolerance
          const min = expectedRuntime - expectedRuntime * tolerance

          assert.ok(diff < max, 'Runtime within maximum expected time')
          assert.ok(diff > min, 'Runtime at least minimum expected time')

          return onDone(true)
        }
      })

      assert.ok(p.parse(input))
    },
    'should have worked': function () {
      assert.ok(true, 'start & stop works')
    }
  },
  'corner cases': {
    'parse empty string': function () {
      const p = new expat.Parser('UTF-8')
      p.parse('')
      assert.ok(true, 'Did not segfault')
    },
    'Escaping of ampersands': function () {
      expect('<e>foo &amp; bar</e>',
        [['startElement', 'e', {}],
          ['text', 'foo & bar'],
          ['endElement', 'e']])
    },
    'parsing twice the same document with the same parser instance should be fine': function () {
      const p = new expat.Parser('UTF-8')
      const xml = '<foo>bar</foo>'
      const result = p.parse(xml)
      assert.ok(result)
      assert.isNull(p.getError())
      p.reset()
      const result2 = p.parse(xml)
      assert.isNull(p.getError())
      assert.ok(result2)
    }
  },
  statistics: {
    'line number': function () {
      const p = new expat.Parser()
      assert.equal(p.getCurrentLineNumber(), 1)
      p.parse('\n')
      assert.equal(p.getCurrentLineNumber(), 2)
      p.parse('\n')
      assert.equal(p.getCurrentLineNumber(), 3)
    },
    'column number': function () {
      const p = new expat.Parser()
      assert.equal(p.getCurrentColumnNumber(), 0)
      p.parse(' ')
      assert.equal(p.getCurrentColumnNumber(), 1)
      p.parse(' ')
      assert.equal(p.getCurrentColumnNumber(), 2)
      p.parse('\n')
      assert.equal(p.getCurrentColumnNumber(), 0)
    },
    'byte index': function () {
      const p = new expat.Parser()
      assert.equal(p.getCurrentByteIndex(), -1)
      p.parse('')
      assert.equal(p.getCurrentByteIndex(), -1)
      p.parse('\n')
      assert.equal(p.getCurrentByteIndex(), 1)
      p.parse(' ')
      assert.equal(p.getCurrentByteIndex(), 2)
    }
  },
  'Stream interface': {
    'read file': {
      topic: function () {
        const p = expat.createParser()
        this.startTags = 0
        p.on('startElement', function (name) {
          log('startElement', name)
          this.startTags++
        }.bind(this))
        this.endTags = 0
        p.on('endElement', function (name) {
          log('endElement', name)
          this.endTags++
        }.bind(this))
        p.on('end', function () {
          this.ended = true
          log('ended')
        }.bind(this))
        p.on('close', function () {
          this.closed = true
          log('closed')
          this.callback()
        }.bind(this))
        p.on('error', function (error) {
          assert.fail('Error', error)
        })

        const mystic = fs.createReadStream(path.join(__dirname, 'mystic-library.xml'))
        mystic.pipe(p)
      },
      'startElement and endElement events': function () {
        assert.ok(this.startTags > 0, 'startElement events at all')
        assert.ok(this.startTags === this.endTags, 'equal amount')
      },
      'end event': function () {
        assert.ok(this.ended, 'emit end event')
      },
      'close event': function () {
        assert.ok(this.closed, 'emit close event')
      }
    }
  }
}).export(module)
