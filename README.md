
DSF（Distributed service framework）
-----
**目标**: 基于C++语言实现的一个简单分布式服务框架示例，包括一个逻辑分发服务实现（类似于Nginx），一个逻辑应用实现

**简介**: 
* 逻辑分发进程logicDispatch的功能类似于Nginx，作为网络接入层，会把相应的网络请求转发至多个相应的逻辑处理服务logicXX。
* 逻辑分发进程和逻辑处理进程之间采用TCP长连接进行通讯，逻辑处理进程主动连接逻辑分发进程。
* 处理了很多细节，比如自动重连功能，消息的幂等性考虑等等。

  
* **支持平台**: x86-64 linux  
* **开发语言**: C++  
* **开发平台**: CentOS release 6.3 
* **linux内核版本**: 2.6.32-279.el6.x86_64 
* **gcc 版本**: 4.4.6
* **[libevent](http://libevent.org/)版本**: 2.0.21


**参考文章**

* [服务端架构中的“网关服务器”](http://blog.51cto.com/yaocoder/1374280)

* [IM系统架构设计之浅见](http://blog.51cto.com/yaocoder/1412029)



