#include"groupmodel.hpp"
#include"db.h"

bool GroupModel::createGroup(Group &group){
    char sql[1024] ={0};
    sprintf(sql,"insert into AllGroup(groupname,groupdesc) values('%s','%s')",group.getName().c_str(),group.getDesc().c_str());
    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);//更新表中数据
        group.setId(mysql_insert_id(mysql.getConnect()));
        return true;
    }
    return false;
}
    //加入群组
void GroupModel::addGroup(int userid,int groupid,string role){
    char sql[1024] ={0};
    sprintf(sql,"insert into GroupUser values(%d,%d,'%s')",groupid,userid,role.c_str());
     MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);//更新表中数据
    }
}
    //查询用户所在的群组消息
vector<Group> GroupModel::query(int userid){
    char sql[1024] = {0};
    sprintf(sql,"select a.id,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on a.id = b.groupid where b.userid =%d",userid);
    //两表查询，找到群组表和群组用户表
    vector<Group> groupVec;//用户加入的所有群组
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr){
            MYSQL_ROW row;
            while((row =mysql_fetch_row(res)) != nullptr){
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    for(Group &group :groupVec){//查找每一个群组中对应的群组成员id
        sprintf(sql,"select a.id a.name,a.state,b.grouprole from user a inner join GroupUser b on b.userid = a.id where b.groupid=%d",group.getId());
        MySQL mysql;
        if(mysql.connect()){
            MYSQL_RES* res = mysql.query(sql);
            if(res != nullptr){
                MYSQL_ROW row;
                while((row =mysql_fetch_row(res)) != nullptr){  
                    GroupUser user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    user.setRole(row[3]);
                    group.getUsers().push_back(user);
                }
                mysql_free_result(res);
            }
        }
    }
    return groupVec;
}
    //根据指定的groupid查询群组用户的id列表
vector<int> GroupModel::queryGroupUsers(int userid,int groupid){
    char sql[1024] = {0};
    sprintf(sql,"select userid from groupuser where groupid = %d and userid !=%d",groupid,userid);
    vector<int> idVec;//记录群组中的成员的用户id
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr){
            MYSQL_ROW row;
            while((row =mysql_fetch_row(res)) != nullptr){  
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
            }
        }
    return idVec;
}
