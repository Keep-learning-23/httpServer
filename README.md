# 这是一个简易的HTTP服务器，可用于浏览器远端访问服务器里的资源。
# 如何编译？
make
# 如何使用？
(1).使用默认ip和端口
cd bin
sudo ./server
(2).使用自定义ip和端口
cd bin
./server [ip] [port]
# 如何访问服务器里的资源
(1)打开浏览器；
(2)输入"http://[ip]:[port]/index.html"，如果看到了测试的页面说明程序运行没问题；#注意替换其中的ip和port
(3)resource里面准备了一些用于访问的文件，例如"./resource/jpg/wallpaper1.jpg"，可以在浏览器里输入"http://[ip]:[port]/jpg/wallpaper1.jpg"，其他资源同理。
# 如何清理中间文件?
make clean
# 如何进行压力测试？
cd test
make
./test [ip] [port] [connection_time]