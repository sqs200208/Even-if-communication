#include"friendmodel.hpp"
#include"db.h"

void FriendModel::insert(int userid,int friendid){
    char sql[1024]  = {0};
    sprintf(sql,"insert into friend values(%d,%d)",userid,friendid);
     MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);//更新数据
    }
}
    //返回好友列表
vector<User> FriendModel::query(int userid){
    char sql[1024] = {0};
    sprintf(sql,"select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid=%d",userid);
    //进行两表查询，根据好友列表的好友id，查找user表中的名字；
    vector<User>vec;
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);//查找
        if(res != nullptr){
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr){//获取所有userid好友的一行数据
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}