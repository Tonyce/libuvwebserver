# uvwebserver

simple webserver base on libuv 

[blog](http://ttang.wesy.club/libuv)

1. 构建一个 tcp-echo-server
2. 解析 http req，并响应 http res
    - 使用 llhttp 做 http parser, nodejs 就使用的这个库

由此可见 Get 是可以传 body 的，至于处不处理，看服务端。也许很多框架碰到 Get 直接把body 忽略了。但在协议层是可以的。

done！

`cc -g -Ilibuv/include -Illhttp/build libuv/.libs/libuv.a llhttp/build/libllhttp.a main.c parser.c  -o uvwebserver`