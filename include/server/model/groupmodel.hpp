#ifndef Groupmodel_H
#define Groupmodel_H
#include"group.hpp"
#include<string>
#include<vector>
class GroupModel
{
public:
    //创建群组
    bool createGroup(Group &group);
    //加入群组
    void addGroup(int userid,int groupid,string role);
    //查询用户所在的群组消息
    vector<Group>query(int userid);
    //根据指定的groupid查询群组用户的id列表
    vector<int> queryGroupUsers(int userid,int groupid);
};

#endif