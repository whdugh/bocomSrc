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

#ifndef SKP_ROAD_XMLVALUE_H
#define SKP_ROAD_XMLVALUE_H

#include "global.h"

/******************************************************************************/
//	描述:智能交通检测系统xml模块,处理xml数据
/******************************************************************************/

  class CSkpRoadXmlValue {
  public:


    enum Type {
      TypeInvalid,
      TypeBoolean,
      TypeInt,
      TypeDouble,
      TypeString,
      TypeDateTime,
      TypeArray,
      TypeStruct
    };
	//数组
    typedef std::vector<CSkpRoadXmlValue> ValueArray;
	//结构体
    typedef std::map<std::string, CSkpRoadXmlValue> ValueStruct;

    //构造原型
    CSkpRoadXmlValue() : _type(TypeInvalid) {}
	//bool 型重载
    CSkpRoadXmlValue(bool value) : _type(TypeBoolean) { _value.asBool = value; }
	//整型重载
    CSkpRoadXmlValue(int value)  : _type(TypeInt) { _value.asInt = value; }
	//双精度重载
    CSkpRoadXmlValue(double value)  : _type(TypeDouble) { _value.asDouble = value; }
	//字符串重载
    CSkpRoadXmlValue(std::string const& value) : _type(TypeString) { _value.asString = new std::string(value); }
	//字符串重载
    CSkpRoadXmlValue(const char* value)  : _type(TypeString){ _value.asString = new std::string(value); }
	//日期重载
    CSkpRoadXmlValue(struct tm* value)  : _type(TypeDateTime){ _value.asTime = new struct tm(*value); }
    //从XML数据构造结构
    CSkpRoadXmlValue(std::string const& xml, int* offset) : _type(TypeInvalid){ if ( ! fromXml(xml,offset)) _type = TypeInvalid; }
    //拷贝构造函数
    CSkpRoadXmlValue(CSkpRoadXmlValue const& rhs) : _type(TypeInvalid) { *this = rhs; }
    //析构
    ~CSkpRoadXmlValue() { invalidate(); }
    //释放空间
    void clear() { invalidate(); }

    //重载操作符 =
    CSkpRoadXmlValue& operator=(CSkpRoadXmlValue const& rhs);
	//重载操作符 =
    CSkpRoadXmlValue& operator=(int const& rhs) { return operator=(CSkpRoadXmlValue(rhs)); }
	//重载操作符 =
	CSkpRoadXmlValue& operator=(bool const& rhs) { return operator=(CSkpRoadXmlValue(rhs)); }
	//重载操作符 =
	CSkpRoadXmlValue& operator=(double const& rhs) { return operator=(CSkpRoadXmlValue(rhs)); }
	//重载操作符 =
	CSkpRoadXmlValue& operator=(const char* rhs) { return operator=(CSkpRoadXmlValue(std::string(rhs))); }
	//重载操作符 ==
    bool operator==(CSkpRoadXmlValue const& other) const;
	//重载操作符 !=
    bool operator!=(CSkpRoadXmlValue const& other) const;
	//重载操作符 &
    operator bool&()          { assertTypeOrInvalid(TypeBoolean); return _value.asBool; }
	//重载操作符 &
    operator int&()           { assertTypeOrInvalid(TypeInt); return _value.asInt; }
	//重载操作符 &
    operator double&()        { assertTypeOrInvalid(TypeDouble); return _value.asDouble; }
	//重载操作符 &
    operator std::string&()   { assertTypeOrInvalid(TypeString); return *_value.asString; }
	//重载操作符 &
    operator struct tm&()     { assertTypeOrInvalid(TypeDateTime); return *_value.asTime; }

	//重载操作符 [int]
    CSkpRoadXmlValue const& operator[](int i) const { assertArray(i+1); return _value.asArray->at(i); }
	//重载操作符 [int]
    CSkpRoadXmlValue& operator[](int i)             { assertArray(i+1); return _value.asArray->at(i); }
	//重载操作符 [string]
    CSkpRoadXmlValue& operator[](std::string const& k) { assertStruct(); return (*_value.asStruct)[k]; }
	//重载操作符 [char*]
    CSkpRoadXmlValue& operator[](const char* k) { assertStruct(); std::string s(k); return (*_value.asStruct)[s]; }

	//数据是否有效
    bool valid() const { return _type != TypeInvalid; }
    //数据类型
    Type const &getType() const { return _type; }

    //返回size
    int size() const;
    //设置数组的长度
    void setSize(int size)    { assertArray(size); }
    //结构体字段
    bool hasMember(const std::string& name) const;

    //从xml生成结构
    bool fromXml(std::string const& valueXml, int* offset);

    //结构生成xml数据
    std::string toXml() const;

    //输出内容
    std::ostream& write(std::ostream& os) const;

  protected:
    //释放
    void invalidate();

    //检测类型
    void assertTypeOrInvalid(Type t);
	//数组
    void assertArray(int size) const;
    void assertArray(int size);

	//结构体
    void assertStruct();

    //xml转换为对应数据

	//xml转换为bool
    bool boolFromXml(std::string const& valueXml, int* offset);
	//xml转换为int
    bool intFromXml(std::string const& valueXml, int* offset);
	//xml转换为double
    bool doubleFromXml(std::string const& valueXml, int* offset);
	//xml转换为字符串
    bool stringFromXml(std::string const& valueXml, int* offset);
	//xml转换为日期
    bool timeFromXml(std::string const& valueXml, int* offset);
	//xml转换为数组
    bool arrayFromXml(std::string const& valueXml, int* offset);
	//xml转换为结构体
    bool structFromXml(std::string const& valueXml, int* offset);

    //各种类型转换为xml格式

	//bool 型转换为xml
    std::string boolToXml() const;
	//整型转换为xml
    std::string intToXml() const;
	//双精度转换为 xml
    std::string doubleToXml() const;
	//字符串转换为xml
    std::string stringToXml() const;
	//日期转换为xml
    std::string timeToXml() const;
	//数组转换为xml
    std::string arrayToXml() const;
	//结构体转换为xml
    std::string structToXml() const;


    //数据类型
    Type _type;

    //数组联合体
    union {
      bool          asBool;		//bool 型
      int           asInt;		//整型
      double        asDouble;	//双精度
      struct tm*    asTime;		//日期
      std::string*  asString;	//字符串
      ValueArray*   asArray;	//数组
      ValueStruct*  asStruct;	//结构体
    } _value;

  };


#endif
