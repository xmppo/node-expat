# node-expat #

## Motivation ##

You use [node.js](http://github.com/ry/node) for speed? You process
XML streams? Then you want the fastest XML parser: libexpat!

## Speed ##

A stupid speed test is supplied in `bench.js`. We measure how many
25-byte elements a SAX parser can process:

* [node-xml](http://github.com/robrighter/node-xml) (pure JavaScript): 23000 el/s
* [libxmljs](http://github.com/polotek/libxmljs) (libxml2 binding): 77000 el/s
* [node-expat](http://github.com/astro/node-expat) (libexpat binding, this): 113000 el/s

These numbers were recorded on a Core 2 2400 MHz and may turn out to
be bullshit, given my few node.js experience.
