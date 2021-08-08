// const net = require("net");

// const data = `{"msg":"Hello World!"}`;
// const buf = Buffer.from(data);
// var socket = net.connect(1234, "localhost", function () {
//   socket.write(
//     "GET /upload?a=b HTTP/1.1\r\n" +
//       "Host: localhost:3000\r\n" +
//       "Content-Type: application/json\r\n" +
//       "Connection: close\r\n" +
//       `Content-Length: ${buf.length}\r\n` +
//       "\r\n" +
//       data
//   );
//   socket.on("data", (chunk) => {
//     console.log(chunk.toString(), data);
//     console.log("--------");
//   });
//   socket.on("end", () => {
//     console.log("--end--");
//   });
// });

// const http = require('http');

// http.get('http://localhost:1234', (res) => {
//     res.on('data', (data) => {
//         console.log(data.toString())
//     })

//     res.on('end', () => {
//     })
// })

const http = require('http');

const postData = JSON.stringify({
  'msg': 'Hello World!'
});

const options = {
  hostname: 'localhost',
  port: 1234,
  path: '/upload',
  method: 'POST',
  headers: {
    'Content-Type': 'application/json',
    'Content-Length': Buffer.byteLength(postData)
  }
};

const req = http.request(options, (res) => {
  console.log(`STATUS: ${res.statusCode}`);
  console.log(`HEADERS: ${JSON.stringify(res.headers)}`);
  res.setEncoding('utf8');
  res.on('data', (chunk) => {
    console.log(`BODY: ${chunk}`);
  });
  res.on('end', () => {
    console.log('No more data in response.');
  });
});

req.on('error', (e) => {
  console.error(`problem with request: ${e.message}`);
});

// Write data to request body
req.write(postData);
req.end();
