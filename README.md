
simple_microservices_framework
-----
**目标**: 基于C++语言实现的一个简单微服务框架示例，包括一个网关（类似于Nginx，承载前端网络层处理、后端微服务调度等），一个后端应用微服务示例

**简介**: 
* 网关服务gatewayServer的功能类似于Nginx，作为网络接入层，会把相应的网络请求按照一定的调度策略转发至多个相应的后端微服务应用appServer1，appServer1……。
* gatewayServer和appServer之间采用TCP长连接进行通讯，appServer主动连接gatewayServer进行注册。
* 添加了很多细节处理，比如自动重连功能，消息的幂等性考虑等等。

  
* **支持平台**: x86-64 linux  
* **开发语言**: C++  
* **开发平台**: CentOS release 6.3 
* **linux内核版本**: 2.6.32-279.el6.x86_64 
* **gcc 版本**: 4.4.6
* **[libevent](http://libevent.org/)版本**: 2.0.21


**参考文章**

* [服务端架构中的“网关服务器”](http://blog.51cto.com/yaocoder/1374280)

* [IM系统架构设计之浅见](http://blog.51cto.com/yaocoder/1412029)



