#ifndef DB_H
#define DB_H
#include<mysql/mysql.h>
#include<string>
using namespace std;
class MySQL
{
public:
    MySQL();
    ~MySQL();
    //连接数据库
    bool connect();
    bool update(string sql);
    MYSQL_RES* query(string sql);//返回结果
    MYSQL* getConnect();
private:
    MYSQL *_conn;
};
#endif