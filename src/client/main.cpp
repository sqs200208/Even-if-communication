#include"json.hpp"
#include<iostream>
#include<thread>
#include<string>
#include<vector>
#include<ctime>
using namespace std;
using json = nlohmann::json;
#include<unistd.h>
#include<sys/socket.h>
#include<semaphore.h>
#include<atomic>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include"group.hpp"
#include"user.hpp"
#include"public.hpp"
//记录当前系统登录的用户信息
User g_currentUser;
//记录当前登录用户的好友列表
vector<User> g_currentUserFriendList;
//记录当前用户的群组的列表信息
vector<Group> g_currentUserGroupList;
//用于读写线程的通信
sem_t rwsem;
atomic_bool g_isLoginSuccess{false};
//显示当前登录成功用户的基本信息
void showCurrentUserData();
//接收线程
void readTaskHandler(int clientfd);
string getCurrentTime();
bool isMainMenuRuing = false;
void mainMenu(int clientfd);

int main(int argc,char **argv)
{
    if(argc<3)//判断命令的个数
    {
        cerr<<"command invalid! example: ./ChatClient 127.0.0.1 6000"<<endl;
        exit(-1);
    }
    //解析IP地址和端口号
    char*ip = argv[1];
    uint16_t port = atoi(argv[2]);
    //创建client的socket
    int clientfd = socket(AF_INET,SOCK_STREAM,0);
    if (-1 == clientfd)
    {
        cerr<<"socket create error"<<endl;
        exit(-1);
    }
    //设置想要连接的服务器的ip地址和端口号
    sockaddr_in server;
    memset(&server,0,sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);
    if(-1 == connect(clientfd,(sockaddr*)&server,sizeof(sockaddr_in)))//对服务器进行连接
    {
        cerr<<"connect server error"<<endl;
        close(clientfd);
        exit(-1);
    }
    sem_init(&rwsem,0,0);//初始化线程通信的信号量
    //登录成功，创建子线程用来接收消息
    std::thread readTask(readTaskHandler,clientfd);//
    readTask.detach();//线程分离

    for(;;)
    {
        //显示首页面菜单 登录，注册，退出
        cout<<"========================"<<endl;
        cout<<"1.login"<<endl;
        cout<<"2.register"<<endl;
        cout<<"3.quit"<<endl;
        cout<<"========================"<<endl;
        int choice = 0;
        cout<<"choice:";
        cin>>choice;//读取整数
        cin.get();//读取掉缓冲区中的回车
        switch (choice)
        {
            case 1://登录业务
            {
                int id = 0;
                char pwd[50] = {0};
                cout<<"userid:";
                cin>>id;
                cin.get();
                cout<<"userpassword:";
                cin.getline(pwd,50);

                json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["password"] = pwd;
                string request = js.dump();
                int len = send(clientfd,request.c_str(),strlen(request.c_str())+1,0);//将序列化的数据发送过去
                if(len == 0)
                {
                    cerr<<"send reg msg error:"<<request<<endl;
                }
                else
                {   
                    g_isLoginSuccess = false;
                    sem_wait(&rwsem);
                    if(g_isLoginSuccess)
                    {
                         isMainMenuRuing = true;
                        //进入主界面
                        mainMenu(clientfd);
                    }
                }
            }
            break;
            case 2://注册业务
            {
                char name[50] = {0};
                char pwd[50] = {0};
                cout<<"username:";
                cin.getline(name,50);
                cout<<"userpassword:";
                cin.getline(pwd,50);

                json js;//封装json
                js["msgid"] = REG_MSG;
                js["name"] = name;
                js["password"] = pwd;
                string request = js.dump();
                cout<<"======="<<endl;
                int len = send(clientfd,request.c_str(),strlen(request.c_str())+1,0);//将序列化的数据发送过去
            
                if(len == -1)
                {
                    cerr<<"send reg msg error:"<<request<<endl;
                }
                else
                {   
                    char buffer[1024] = {0};
                    len = recv(clientfd,buffer,1024,0);//阻塞等待
                    if(len == -1)
                    {
                        cerr<<"recv reg response error"<<endl;
                    }
                }
                sem_wait(&rwsem);//阻塞等待


            }
            break;
            case 3://退出业务
            {
                close(clientfd);
                sem_destroy(&rwsem);//释放
                exit(0);
            }
            default:
                cerr<<"invalid input:"<<endl;
                break;
        }
    }
    return 0;
}

void doREGRespone(json &responsejs)
{
    cout<<"----------------------------"<<endl;
    if(0 != responsejs["errno"].get<int>())
    {
        cerr<<"is already exist,register error!"<<endl;
    }
    else
    {
        cout<<"register success,userid is:"<<responsejs["id"]<<",do not forgtw it!"<<endl;
    }

}
void doLoginRespone(json &responsejs)
{
    
    if(0!=responsejs["errno"].get<int>())
    {
        cerr<<responsejs["errmsg"]<<endl;
        g_isLoginSuccess = false;
    }
    else
    {
        //记录当前用户id和name
        g_currentUser.setId(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);
        //记录当前好友的列表
        if(responsejs.contains("friend"))
        {
            vector<string> vec = responsejs["friends"];
            for(string &str:vec)
            {
                g_currentUserFriendList.clear();
                json js = json::parse(str);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                g_currentUserFriendList.push_back(user);
            }
        }
        //记录当前用户的群组列表信息
        if(responsejs.contains("groups"))
        {
            vector<string> vec1 = responsejs["groups"];
            for(string & groupstr:vec1)
            {
                g_currentUserGroupList.clear();
                json grpjs = json::parse(groupstr);
                Group group;
                group.setId(grpjs["id"].get<int>());
                group.setName(grpjs["groupname"]);
                group.setDesc(grpjs["groupsesc"]);
                vector<string> vec2= grpjs["users"];
                for(string &userstr:vec2)
                {
                    GroupUser user;
                    json js = json::parse(userstr);
                    user.setId(js["id"].get<int>());
                    user.setName(js["name"]);
                    user.setState(js["state"]);
                    user.setRole(js["role"]);
                    group.getUsers().push_back(user);
                }
                g_currentUserGroupList.push_back(group);
            }
        }
        showCurrentUserData();
        //显示当前用户的离线消息
        if(responsejs.contains("offlinemsg"))
        {
            vector<string> vec = responsejs["offlinemsg"];
            for(string &str:vec)
            {
                json js = json::parse(str);
                if(ONE_CHAT_MSG==js["msgid"].get<int>())
                {
                    cout<<js["time"].get<string>()<<"["<<js["id"]<<"]"<<js["name"].get<string>()<<"said:"<<js["msg"].get<string>()<<endl;
                }
                else if(js["msgid"].get<int>()==GROUP_CHAT_MSG)
                {
                    cout<<"group:["<<js["groupid"]<<"]"<<js["time"].get<string>()<<"["<<js["id"]<<"]"<<js["name"].get<string>()<<"said:"<<js["msg"].get<string>()<<endl;                      
                }
            
            }
        }
        g_isLoginSuccess = true;
    }
}


void showCurrentUserData()
{

    cout<<"======================login user================"<<endl;
    cout<<"current login user=>id:"<<g_currentUser.getId()<<"name:"<<g_currentUser.getName()<<endl;
    cout<<"---------------------------friend list------------"<<endl;
    if(!g_currentUserFriendList.empty())
    {
        for(User &user:g_currentUserFriendList)
        {
            cout<<user.getId()<<" "<<user.getName()<<" "<<user.getState()<<endl;
        }
        if(!g_currentUserGroupList.empty())
        {
            for(Group & group :g_currentUserGroupList)
            {
                cout<<group.getId()<<" "<<group.getName()<<" "<<group.getDesc()<<endl;
                for(GroupUser &user:group.getUsers())
                {
                    cout<<user.getId()<<" "<<user.getName()<< " "<<user.getState()<<" "<<user.getRole()<<endl;
                }

            }
        }
    }
    cout<<"-------------------------------------------"<<endl;
}
void readTaskHandler(int clientfd)
{
    for(;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd,buffer,1024,0);
        if(len ==0 || len==-1)
        {
            close(clientfd);
            exit(-1);
        }
        //josn的反序列化
        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();
        if(ONE_CHAT_MSG==msgtype)
        {
            cout<<js["time"].get<string>()<<"["<<js["id"]<<"]"<<js["name"].get<string>()<<"said:"<<js["msg"].get<string>()<<endl;
            continue;
        }
        else if(msgtype==GROUP_CHAT_MSG)
        {
            cout<<"group:["<<js["groupid"]<<"]"<<js["time"].get<string>()<<"["<<js["id"]<<"]"<<js["name"].get<string>()<<"said:"<<js["msg"].get<string>()<<endl;
            continue;
        }
        if(LOGIN_MSG_ACK==msgtype)
        {
            doLoginRespone(js);//处理登录响应业务
            sem_post(&rwsem);
            continue;
        }
        if(REG_MSG_ACK==msgtype)
        {
            doREGRespone(js);
            sem_post(&rwsem);
            continue;
        }
        
    }
}
void help(int fd = 0,string str = "");
void chat(int ,string);
void addfriend(int,string);
void creategroup(int,string);
void addgroup(int,string);
void groupchat(int ,string);
void loginout(int,string);
unordered_map<string ,string> commandMap ={
    {"help","显示所有支持的命令,格式help"},
    {"chat","一对一聊天,格式chat:friendid:message"},
    {"addfriend","添加好友,格式creategroup:groupname:groupdesc"},
    {"addgroup","加入群组,格式addgroup:groupid"},
    {"groupchat","群聊,格式groupchat:groupid:message"},
    {"loginout","注销,格式loginout"}
};
unordered_map<string,function<void(int,string)>>commandHandlerMap = {
    {"help",help},
    {"chat",chat},
    {"addfriend",addfriend},
    {"creategroup",creategroup},
    {"addgroup",addgroup},
    {"groupchat",groupchat},
    {"loginout",loginout}
};

void mainMenu(int clientfd)
{   
    help();
    char buffer[1024] = {0};
    while (isMainMenuRuing)
    {
        cin.getline(buffer,1024);//读取一行数据
        string commandbuf(buffer);
        string command;//存储命令
        int idx = commandbuf.find(":");//找到带：的命令，并返回下标
        if(idx ==-1)
        {
            command = commandbuf; //若没找到，就直接等于输入
        }
        else
        {
            command = commandbuf.substr(0,idx);//分离
        }
        auto it = commandHandlerMap.find(command);
        if(it == commandHandlerMap.end())
        {
            cerr<<"invalid input command!"<<endl;
            continue;
        }
        it->second(clientfd,commandbuf.substr(idx+1,commandbuf.size()-idx));

    }
}

void help(int fd,string str)
{
    cout<<"show command list>>>"<<endl;
    for(auto &p:commandMap)
    {
        cout<<p.first<<":"<<p.second<<endl;
    }
    cout<<endl;
}

void chat(int clientfd,string str)
{
    int idx = str.find(":");
    if(idx == -1)
    {
        cerr<<"chat command invalidl"<<endl;
        return;
    }
    int friendid = atoi(str.substr(0,idx).c_str());
    string message = str.substr(idx+1,str.size() - idx);
    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    cout<<"-------------------------------------------------------"<<endl;
    if(len == -1)
    {
        cerr<<"send chat msg error->"<<buffer<<endl;
    }
}
void addfriend(int clientfd,string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len == -1)
    {
        cerr<<"send addfriend msg errror ->"<<buffer<<endl;
    }
}
string  getCurrentTime()
 {
    auto tt=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm=localtime(&tt);
    char date[128]={0};
    sprintf(date,"%d-%02d-%02d %02d:%02d:%02d",
        (int)ptm->tm_year+1900,(int)ptm->tm_mon+1,(int)ptm->tm_mday,
        (int)ptm->tm_hour,(int)ptm->tm_min,(int)ptm->tm_sec);
    return std::string(date);
 }
void creategroup(int clientfd,string str)
{
    int idx = str.find(":");
    if(idx == -1)
    {
        cerr<<"creategroup command invalid!"<<endl;
        return;
    }
    string groupname = str.substr(0,idx);
    string groupdesc = str.substr(idx+1,str.size()-idx);
    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len == -1)
    {
        cerr<<"send creategroup msg error->"<<buffer<<endl;
    }
}
void addgroup(int clientfd,string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len == -1)
    {
        cerr<<"send addgroup msg error->"<<buffer<<endl;
    }
}
void groupchat(int clientfd,string str)
{
    int idx = str.find(":");
    if(idx ==-1)
    {
        cerr<<"groupchat command invalid!"<<endl;
        return ;
    }
    int groupid = atoi(str.substr(0,idx).c_str());;
    string message = str.substr(idx+1,str.size()-idx);
    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len == -1)
    {
        cerr<<"send groupchat msg error->"<<buffer<<endl;
    }


}
void loginout(int clientfd,string)//用户注销功能
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len == -1)
    {
        cerr<<"send loginout msg error->"<<buffer<<endl;
    }
    else
    {
        isMainMenuRuing = false;
    }

}