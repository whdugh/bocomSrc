#ifndef MYSQLTRANSITIONSQLITECYTOUSB_H
#define MYSQLTRANSITIONSQLITECYTOUSB_H

/*
	这个类主要功能是： 把mysql里面的数据copy到 sqlite 里面 然后再 把sqlite3 类型的数据包epolice.db
	copy到 u盘中
	
*/

#include <math.h>
#include "Common.h"
#include "global.h"
#include <sys/stat.h>
#include <sys/mount.h>
#include <string>
#include <iconv.h>
#include <stdio.h>
#include <memory.h>

#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/ioctl.h> 
#include <sys/types.h> 
#include <linux/kd.h> 
#include "/usr/include/mysql/mysql.h"


//#define SQLITE_OK
  
#ifdef SQLITE_OK
#include "sqlite3.h"

using namespace std;

class MysqlTransitionSqlteCyToUSB
{
public:
	MYSQL  * m_mysql;
	sqlite3 * m_db;
	pthread_mutex_t m_sqlite3_mutex;

	iconv_t cd;
	
	//pthread_t m_nThreadId;


	MysqlTransitionSqlteCyToUSB();
	~MysqlTransitionSqlteCyToUSB();

	int openSqlite();                                     // 打开sqlite3数据库

	int  AddField();

	int  createTablePassVehicle();                        // 创建 passVehicle 表 

	int  deleteTablePassVehicle();                        // 删除passVehicle 表
	
	bool intThreadMonitorFindUSB();						  // 这个启动一个检测发现u盘的线程
	
	void RunToCopy();                                     // 这个方法是启动任务（启动一个线程， 这个线程不断的检测是否有u盘插入如果有发现都会进行做copy的工作）

	int fileWriteUSB();                                   // 把这个db文件copy到u盘上面

	void closeSqlite();							          // 关闭sqlite3数据库         

 	int insetIntosqliteMessage(string str);         // 把数据插入到sqlite3 里面

	int execsqlInsqlite(string strSql);		// 在sqlite3 中执行sql语句

	int insetIntosqliteMessage(string sqlPaht, string strSql);                                           

	char * transitionData(MYSQL_ROW row);                                  // 这个函数的功能能是： 把mysql里面的数据重新排序组合成插入sqlite3的sql 语句

	int carLabel(int number);    //车标

	int carColor(int number);    //车身颜色
	
	int wfxw(int number);       //检测结果类型

	int carType(int number, string str);  //车的类型

	int wflx(int number);

	void CodeConverterOpen(const char *from_charset,const char *to_charset) 
	{
			 cd = iconv_open(to_charset,from_charset);
	}

		
	void CodeConverterClose() 
	{
			iconv_close(cd);
	}

		// 转换输出	
	int converting(char *inbuf,int inlen,char *outbuf,int outlen) 
	{
			char **pin = &inbuf;
			char **pout = &outbuf;

			memset(outbuf,0,outlen);
			return iconv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen);
	}

protected:


private:


};




//void * findUSBThread(void *);                              // 这个线程是发现有没有usb 插进去



#endif
#endif
