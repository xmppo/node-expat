var util = require('util'),
    fs = require('fs'),
    expat = require('./lib/node-expat');

var p = expat.createParser();
var mystic = fs.createReadStream(__dirname + '/mystic-library.xml');
mystic.pipe(p);
p.on('startElement', function(name, attr) {
  console.log(name + ' ' + attr)
})
