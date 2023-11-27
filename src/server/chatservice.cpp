#include"chatservice.hpp"
#include"public.hpp"
#include<muduo/base/Logging.h>
#include<map>
using namespace muduo;
using namespace std;
ChatService::ChatService(){
    //用户的基本事件
    _msgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::login,this,_1,_2,_3)});
    _msgHandlerMap.insert({REG_MSG,std::bind(&ChatService::reg,this,_1,_2,_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::oneChat,this,_1,_2,_3)});
    _msgHandlerMap.insert({LOGINOUT_MSG,std::bind(&ChatService::loginout,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend,this,_1,_2,_3)});

    //群组业务管理相关事件处理回调函数
    _msgHandlerMap.insert({CREATE_GROUP_MSG,std::bind(&ChatService::createGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG,std::bind(&ChatService::groupChat,this,_1,_2,_3)});
}

ChatService*  ChatService::instance()
{
    static ChatService service;
    return &service;
}

MsgHandler ChatService::getHandler(int msgid)//返回对应处理的业务
{
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr &conn,json js,Timestamp){
            LOG_ERROR<<"msgid:"<<msgid<<"can not find handler";
        };
    }
    return _msgHandlerMap[msgid];
}
//处理登录
void ChatService::login(const TcpConnectionPtr &conn,json js,Timestamp time)
{
    int id = js["id"].get<int>();
    string pwd = js["password"];
    User user = _userModel.query(id);//通过数据操作对象返回一个，数据表对象
    if(user.getId()==id && user.getPwd() == pwd)
    {
        if(user.getState() == "online")
        {
            //用户重复登陆
            json response;
            response["msgid"] = LOGIN_MSG_ACK;//回应一个ack
            response["errno"] = 2;//出错
            response["errmsg"] ="用户重复登陆";
            conn->send(response.dump());//发送给另一端
        }
        else
        {
            //记录用户的连接,使用线程安全的方法进行调用
            {
                lock_guard<mutex>lock(_connMutex);
                _userConnMap.insert({id,conn});
            }
            //更新用户状态信息，登录成功
            user.setState("online");
            _userModel.updateState(user);
            json response;
            response["msgid"] = LOGIN_MSG_ACK;//回应一个ack
            response["errno"] = 0;//没有出错
            response["id"] = user.getId();//得到生成的id
            response["name"] = user.getName();
            //读取离线消息
            vector<string> vec = _OfflineMsgModel.query(id);
            if(!vec.empty())
            {
                response["offlinemsg"] = vec;
                _OfflineMsgModel.remove(id);
            } 
            //查询好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty())
            {
                vector<string> vec2;
                for(User &user:userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friend"] = vec2;
            }
           
            conn->send(response.dump());//发送给另一端
        }
    }
    else
    {
        json response;
        response["msgid"] = REG_MSG_ACK;//回应一个ack
        response["errno"] = 1;//出错
        response["errmsg"] = "用户名密码错误";
        conn->send(response.dump());//发送给另一端

    }
}
//处理注册
void ChatService::reg(const TcpConnectionPtr &conn,json js,Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if(state)
    {
        //注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;//回应一个ack
        response["errno"] = 0;//没有出错
        response["id"] = user.getId();//得到生成的id
        conn->send(response.dump());//发送给另一端

    }
    else
    {
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;//表示注册失败，也就不需要生成id了
        conn->send(response.dump());
    }

}
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for(auto it = _userConnMap.begin();it != _userConnMap.end();++it)
        {
            if(it->second == conn)
            {
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }

    }
    if(user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

void ChatService::oneChat(const TcpConnectionPtr &conn,json js,Timestamp time)
{
    int toid =js["to"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if(it != _userConnMap.end())
        {
            //用户在线，发送消息
            it->second->send(js.dump());
            return;
        }
    }
    //用户不在线，存储消息；
    _OfflineMsgModel.insert(toid,js.dump());
}
//设置状态
void ChatService::reset()
{
    _userModel.resetState();
}
//添加好友
void ChatService::addFriend(const TcpConnectionPtr &conn,json js,Timestamp time)
{
    int userid = js["id"].get<int>();//用户id
    int friendid = js["friendid"].get<int>();//添加的朋友id
    _friendModel.insert(userid,friendid);//添加到数据库中
}
//创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn,json js,Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    Group group(-1,name,desc);
    if(_groupModel.createGroup(group))
    {
        _groupModel.addGroup(userid,group.getId(),"creator");
    }
}
//加入群组
void ChatService::addGroup(const TcpConnectionPtr &conn,json js,Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid,groupid,"normal");
}
//群组聊天
void ChatService::groupChat(const TcpConnectionPtr &conn,json js,Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int>useridVec = _groupModel.queryGroupUsers(userid,groupid);//查找组员的id
    lock_guard<mutex> lock(_connMutex);
    for(int id:useridVec)//对每一个组员进行转发消息
    {
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end())//如果在线直接转发
        {
            it->second->send(js.dump());
        }
        else//若不在线，进行离线消息存储
        {
            _OfflineMsgModel.insert(id,js.dump());
        }
    }
}
void ChatService::loginout(const TcpConnectionPtr &conn,json js,Timestamp time)
{
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if(it!=_userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }
    User user(userid," "," ","offline");
    _userModel.updateState(user);
}