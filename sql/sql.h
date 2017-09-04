#ifndef _SQL_H_
#define _SQL_H_

#include<string.h>
#include<iostream>
#include<string>
#include<mysql.h>
#include<stdlib.h>
  class sql{
  	public:
  		sql(const std::string &_user,\
				const std::string &_ip,\
        	    const std::string &_passwd,\
  		    const std::string &_db,\
  		    const int &_port);

  	int connect();
    int  insert(const std::string &name ,\
  		  const std::string &sex,\
    		  const std::string &school,\
    		  const std::string &hobby);
  	int select();
	~sql();
  	private:
  		MYSQL *conn;
  		std::string user;
  		std::string ip;
  		std::string passwd;
  		std::string db;
  		int port;
  };
  
  
#endif
