// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

//MysqlQuery.h
#ifndef MYSQLQUERY_H
#define	MYSQLQUERY_H

#include <stdio.h>
#include <string>
#include "/usr/include/mysql/mysql.h"

using namespace std;

class MysqlQuery
{
	public:
	
	MysqlQuery();
	MysqlQuery(bool beof);//空集合
	MysqlQuery(MYSQL_RES *query);
	MysqlQuery(MYSQL *psql,MYSQL_RES *query,MYSQL_ROW rr,bool beof);
	

	~MysqlQuery();

	public:

		MysqlQuery& operator=(const MysqlQuery& rQuery);

		int numFileds();//结果集数量
		string getStringFileds(int nIndex);//获得字符串字段
		int getIntFileds(int nIndex);//获得整型字段
		unsigned int getUnIntFileds(int nIndex);//获得整型字段
		long getLongFileds(int nIndex);//获得long字段
		float getFloatFileds(int nIndex);//获得Float字段

		string getStringFileds(string name);//获得字符串字段
		int getIntFileds(string name);//获得整型字段
		unsigned int getUnIntFileds(string name);//获得整型字段
		long getLongFileds(string name);//获得long字段
		float getFloatFileds(string name);//获得Float字段

				
		void nextRow();//下一行
		bool eof();//结果集为空

		void finalize();//释放结果集

		MYSQL_RES * getSelectResult(); // 的到查询的结果


	private:

		MYSQL *mysql;//mysql连接
		MYSQL_RES *result;//查询结果集合
		MYSQL_ROW row;//结果集合一条记录
		
		int m_nRows;//结果集合的最大行数
		int m_nCols;//结果集合的最大烈数

		bool mbEof;//结果集已经为空

		map<string, int> m_fieldMap;//保存表头中字段名的map
};
#endif
