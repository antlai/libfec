var fec = require('./fec.js');

var cleanup = fec.cwrap('cleanup', null, []);
var encode = fec.cwrap('encode', 'number', ['number', 'number']);
var decode = fec.cwrap('decode', 'number', ['number', 'number', 'number',
                                            'number']);
var NUM_SYMBOLS=20;
var NUM_DATA=8;

var data = fec._malloc(NUM_SYMBOLS);

var erasure = fec._malloc(NUM_SYMBOLS);

var dataArray = fec.HEAPU8.subarray(data, data + NUM_SYMBOLS);

var erasureArray = fec.HEAPU8.subarray(erasure, erasure + NUM_SYMBOLS);

dataArray[0] = 8;
for (var i=1; i< NUM_DATA; i++) {
    dataArray[i] = (i+1) % NUM_DATA;
}


encode(data, NUM_SYMBOLS);

console.log(dataArray.toString());

var id = decode(data, NUM_SYMBOLS, erasure, 0);
console.log(id);

console.log('Adding errors...');
// Can recover 12 erasures
dataArray[0] = 0;

dataArray[1] = 0;
dataArray[2] = 0;
dataArray[3] = 0;
dataArray[4] = 0;

dataArray[5] = 0;

dataArray[6] = 0;
dataArray[7] = 0;

dataArray[8] = 0;
dataArray[9] = 0;
dataArray[10] = 0;
dataArray[11] = 0;

erasureArray.set([0,1,2,3,4,5, 6,7,8,9,10,11], 0);
id = decode(data, NUM_SYMBOLS, erasure, 12);
console.log(id);

// can recover 6 errors
dataArray[0] = 0;

dataArray[1] = 0;
dataArray[2] = 0;
dataArray[3] = 0;
dataArray[4] = 0;

dataArray[5] = 0;

id = decode(data, NUM_SYMBOLS, erasure, 0);
console.log(id);

console.log(dataArray.toString());

// can't recover 7 errors
dataArray[0] = 0;
dataArray[1] = 0;
dataArray[2] = 0;
dataArray[3] = 0;
dataArray[4] = 0;
dataArray[5] = 0;
dataArray[15] = 0;
id = decode(data, NUM_SYMBOLS, erasure, 0);
console.log(id);
console.log(dataArray.toString());

// but we can recover 5 errors and 2 erasures
id = decode(data, NUM_SYMBOLS, erasure, 2);
console.log(id);
console.log(dataArray.toString());

fec._free(data);
fec._free(erasure);
cleanup();
