const http = require('http');

http.createServer((req, res) => {
    res.end("helloworld")
}).listen(1234)