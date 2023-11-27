#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include <unordered_map>
#include <functional>
#include<muduo/net/TcpConnection.h>
#include<mutex>
#include"offlinemessagemodel.hpp"
#include"json.hpp"
#include"usermodel.hpp"
#include"friendmodel.hpp"
#include "groupmodel.hpp"
using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;
using MsgHandler = std::function<void(const TcpConnectionPtr &conn,json js,Timestamp time)>;
//聊天服务器业务类
class ChatService
{
public:
    static ChatService* instance();
    void login(const TcpConnectionPtr &conn,json js,Timestamp time);
    void reg(const TcpConnectionPtr &conn,json js,Timestamp time);
    void oneChat(const TcpConnectionPtr &conn,json js,Timestamp time);
    void clientCloseException(const TcpConnectionPtr &conn);
    MsgHandler getHandler(int msgid);
    void addFriend(const TcpConnectionPtr &conn,json js,Timestamp time);
    void createGroup(const TcpConnectionPtr &conn,json js,Timestamp time);
    void addGroup(const TcpConnectionPtr &conn,json js,Timestamp time);
    void groupChat(const TcpConnectionPtr &conn,json js,Timestamp time);
    void loginout(const TcpConnectionPtr &conn,json js,Timestamp time);
    void reset();
private:
    ChatService();
    unordered_map<int,MsgHandler> _msgHandlerMap;//存储id和其对应的业务
    unordered_map<int,TcpConnectionPtr> _userConnMap;//存储在线的用户连接
    mutex _connMutex;//定义锁来保证线程安全
    UserModel _userModel;//数据操作对象，用来操作数据库，只提供了一个类中的一个对象在该类中只能操作这个对象，使用这个对象来调用相应的方法；
    OfflineMsgModel _OfflineMsgModel;//离线消息操作对象
    FriendModel _friendModel;//朋友数据库操作的对象
    GroupModel _groupModel;//群组操作的对象
};
#endif
