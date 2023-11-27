#include"chatserver.hpp"
#include<functional>
#include"json.hpp"
#include<string>
#include"chatservice.hpp"
using namespace std;
using namespace placeholders;
using json = nlohmann::json;
 Chatserver::Chatserver(EventLoop *loop,const InetAddress &listenAddr,const string &nameArg):_server(loop,listenAddr,nameArg), _loop(loop)
 {
    _server.setConnectionCallback(std::bind(&Chatserver::onCOnnection,this,_1));
    _server.setMessageCallback(std::bind(&Chatserver::onMessage,this,_1,_2,_3));
    _server.setThreadNum(4);//创建线程
 }

void Chatserver::start()
{
    _server.start();//运行muduo库，构建的网络模块
}
    //处理用户连接相关信息的回调函数
void Chatserver::onCOnnection(const TcpConnectionPtr &conn)
{
    if(!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}
//处理读写事件相关信息的回调函数
void Chatserver::onMessage(const TcpConnectionPtr &conn,Buffer *buffer,Timestamp time)
{
    string buf = buffer->retrieveAllAsString();//数据的反序列化
    json js = json::parse(buf);//可以得到conn，time，需要处理的业务（根据js得到）
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());//返回处理的业务 
    //回调消息绑定好的事件处理
    msgHandler(conn,js,time);
}



