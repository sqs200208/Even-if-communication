#ifndef CHATSERVER_H
#define CHATSERVER_H
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class Chatserver
{
public:
    //初始化server
    Chatserver(EventLoop *loop,const InetAddress &listenAdde,const string &nameArg);//监听事件，IP地址端口号，名称
    //启动服务器
    void start();
private:
    //处理用户连接相关信息的回调函数
    void onCOnnection(const TcpConnectionPtr &);
    //处理读写事件相关信息的回调函数
    void onMessage(const TcpConnectionPtr &,Buffer *buffer,Timestamp time);
    TcpServer _server;//组合的muduo库，实现服务器功能的类对象
    EventLoop *_loop;//指向事件循环对象的指针
};
#endif
