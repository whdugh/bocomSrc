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

//MysqlQuery.cpp
#include "Common.h"
#include "MysqlQuery.h"

MysqlQuery::MysqlQuery()
{

	row=NULL;
	result=NULL;
	m_nRows=0;
	m_nCols=0;

	mbEof=true;

	return ;
}
//空集合
MysqlQuery::MysqlQuery(bool beof)
{
	mbEof=beof;
	return;
}

MysqlQuery::MysqlQuery(MYSQL_RES *query)
{
	result=query;
	if(result!=NULL)
	{
		row=mysql_fetch_row(result);
		//获得结果集合的行数
		m_nRows=mysql_num_rows(result);
		m_nCols=mysql_num_fields(result);

		mbEof=false;

		//保存表头中的字段名
		MYSQL_FIELD* field = NULL;
		for (int i = 0; i < m_nCols; i++)
		{
			field = mysql_fetch_field_direct(result, i);
			if (field && field->name)
			{
				m_fieldMap.insert(make_pair(field->name, i));
			}
		}
	}

	return;
}

MysqlQuery::MysqlQuery(MYSQL *psql,MYSQL_RES *query,MYSQL_ROW rr,bool beof)
{
	mysql=psql;
	result=query;
	row=rr;

	mbEof=beof;
}

MysqlQuery::~MysqlQuery()
{

}


MysqlQuery& MysqlQuery::operator=(const MysqlQuery& rQuery)
{
	result=rQuery.result;
	row=rQuery.row;
	mbEof=rQuery.mbEof;

	return *this;
}

//获得返回集合数
int MysqlQuery::numFileds()
{
	//集合为空
	if(result==NULL)
		return 0;

	int nRet = mysql_num_rows(result);
	return nRet;
}

//获得String字段
string MysqlQuery::getStringFileds(int nIndex)
{
	//行集为空
	if(row==NULL)
		//return NULL;
		return "";

	if(nIndex < 0 ||nIndex > m_nCols)
	{
		printf("Index out of Cols!");
		return "";
	}

	string strRet("");

	if(row[nIndex])
		strRet = row[nIndex];

	return strRet;
}

//获得String字段
string MysqlQuery::getStringFileds(string name)
{
	if (name == "")
		return "";

	int index = -1;
	map<string,int>::iterator it = m_fieldMap.find(name);
	if ( it != m_fieldMap.end() )
		index = it->second;

	return getStringFileds(index);
}


//获得Int字段
int MysqlQuery::getIntFileds(int nIndex)
{
	//行集为空
	if(row==NULL)
		return 0;

	int nRet=0;
	if(nIndex < 0 ||nIndex > m_nCols)
	{
		printf("Index out of Cols! m_nCols = %d\r\n",m_nCols);
		return nRet;
	}
	string temp="";

	if(row[nIndex])
		temp = row[nIndex];

    if (!temp.empty())
    {
        nRet=atoi(temp.c_str());
    }

	return nRet;
}

//获得Int字段
int MysqlQuery::getIntFileds(string name)
{
	if (name == "")
		return -1;

	int index = -1;
	map<string,int>::iterator it = m_fieldMap.find(name);
	if ( it != m_fieldMap.end() )
		index = it->second;
	return getIntFileds(index);
}


//获得Int字段
unsigned int MysqlQuery::getUnIntFileds(int nIndex)
{
	//行集为空
	if(row==NULL)
		return 0;

	unsigned int nRet = 0;
	if(nIndex < 0 ||nIndex > m_nCols)
	{
		printf("Index out of Cols!\r\n");
		return nRet;
	}
	string temp("");

	if(row[nIndex])
		temp = row[nIndex];

    if (!temp.empty())
    {
    //	printf("temp.c_str()=%s\r\n",temp.c_str());
        nRet=(unsigned int)(strtoul(temp.c_str(),NULL,0));
    }

	return nRet;
}

unsigned int MysqlQuery::getUnIntFileds(string name)
{
	if (name == "")
		return 0;

	int index = -1;
	map<string,int>::iterator it = m_fieldMap.find(name);
	if ( it != m_fieldMap.end() )
		index = it->second;
	return getUnIntFileds(index);
}

//获得Long字段
long MysqlQuery::getLongFileds(int nIndex)
{
	//行集为空
	if(row==NULL)
		return 0;

	long nRet=-1;
	if(nIndex < 0 ||nIndex > m_nCols)
	{
		printf("Index out of Cols!");
		return nRet;
	}
	string temp="";
	if(row[nIndex])
		temp = row[nIndex];

    if (!temp.empty())
    {
    //	printf("temp.c_str()=%s\r\n",temp.c_str());
        nRet=atol(temp.c_str());
    }

	return nRet;
}

//获得Long字段
long MysqlQuery::getLongFileds(string name)
{
	if (name == "")
		return -1;

	int index = -1;
	map<string,int>::iterator it = m_fieldMap.find(name);
	if ( it != m_fieldMap.end() )
		index = it->second;
	return getLongFileds(index);
}

//获得Float字段
float MysqlQuery::getFloatFileds(int nIndex)
{
	//行集为空
	if(row==NULL)
		return 0;

	float nRet=0;
	if(nIndex < 0 ||nIndex > m_nCols)
	{
		printf("Index out of Cols!");
		return nRet;
	}
	string temp="";
	if(row[nIndex])
		temp = row[nIndex];

    if (!temp.empty())
    {
    //	printf("temp.c_str()=%s\r\n",temp.c_str());
        nRet=atof(temp.c_str());
    }

	return nRet;
}

//获得Float字段
float MysqlQuery::getFloatFileds(string name)
{
	if (name == "")
		return -1;

	int index = -1;
	map<string,int>::iterator it = m_fieldMap.find(name);
	if ( it != m_fieldMap.end() )
		index = it->second;
	return getFloatFileds(index);
}
//下一行
void MysqlQuery::nextRow()
{
	//结果集为空
	if(result==NULL)
	{
		mbEof=true;
		return;
	}

	//行集为空
	row= mysql_fetch_row(result);

	if(row==NULL)
		mbEof=true;


	return;
}

//结果集为空
bool MysqlQuery::eof()
{
	//结集为空
	if(result==NULL)
		return true;

	return mbEof;
}

//释放结果集
void MysqlQuery::finalize()
{
	//空集合
	if(result==NULL)
	{
#ifdef _DEBUG
		printf("already finalize \n");
#endif
		result=NULL;
		return;
	}
	//printf("finalize \n");
	mysql_free_result(result);
	//printf("finalize \n");
	result=NULL;
	return;
}
