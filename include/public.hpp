
/*
server和client公共文件
*/
//登录1，登录回应2,注册3，注册回应4,发送消息5,添加朋友6，创建群组7，加入群组8.群聊天9,注销10
enum EnMsgType
{
    LOGIN_MSG = 1, //登录1
    LOGIN_MSG_ACK,//登录回应2
    REG_MSG,//注册3
    REG_MSG_ACK,//注册回应4
    ONE_CHAT_MSG,//一对一发送消息5
    ADD_FRIEND_MSG,//添加朋友6
    CREATE_GROUP_MSG,//创建群组7
    ADD_GROUP_MSG,//加入群组8
    GROUP_CHAT_MSG,//群聊天9
    LOGINOUT_MSG,//注销10
};
