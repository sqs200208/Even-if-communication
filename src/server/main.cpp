#include"chatserver.hpp"
#include<iostream>
#include<unordered_map>
#include<signal.h>
#include"chatservice.hpp"
using namespace std;
//处理服务器ctrl+C，结束后重置user的状态
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}
int main()
{
    signal(SIGINT,resetHandler);
    unordered_map<int,int> _msgHandlerMap;
    EventLoop loop;
    InetAddress addr("127.0.0.1",6000);
    Chatserver server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();
    return 0;
}