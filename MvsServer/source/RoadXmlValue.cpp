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

#include "Common.h"
#include "RoadXmlValue.h"
#include "RoadXmlUtil.h"


  static const char VALUE_TAG[]     = "<value>";
  static const char VALUE_ETAG[]    = "</value>";

  static const char BOOLEAN_TAG[]   = "<boolean>";
  static const char BOOLEAN_ETAG[]  = "</boolean>";
  static const char DOUBLE_TAG[]    = "<double>";
  static const char DOUBLE_ETAG[]   = "</double>";
  static const char INT_TAG[]       = "<int>";
  static const char I4_TAG[]        = "<i4>";
  static const char I4_ETAG[]       = "</i4>";
  static const char STRING_TAG[]    = "<string>";
  static const char STRING_ETAG[]   = "</string>";

  static const char DATETIME_TAG[]  = "<dateTime.iso8601>";
  static const char DATETIME_ETAG[] = "</dateTime.iso8601>";

  static const char ARRAY_TAG[]     = "<array>";
  static const char DATA_TAG[]      = "<data>";
  static const char DATA_ETAG[]     = "</data>";
  static const char ARRAY_ETAG[]    = "</array>";

  static const char STRUCT_TAG[]    = "<struct>";
  static const char MEMBER_TAG[]    = "<member>";
  static const char NAME_TAG[]      = "<name>";
  static const char NAME_ETAG[]     = "</name>";
  static const char MEMBER_ETAG[]   = "</member>";
  static const char STRUCT_ETAG[]   = "</struct>";

  // Clean up
  void CSkpRoadXmlValue::invalidate()
  {
    switch (_type) {
      case TypeString:    delete _value.asString; break;
      case TypeDateTime:  delete _value.asTime;   break;
      case TypeArray:     delete _value.asArray;  break;
      case TypeStruct:    delete _value.asStruct; break;
      default: break;
    }
    _type = TypeInvalid;
  }

  
  // Type checking
  void CSkpRoadXmlValue::assertTypeOrInvalid(Type t)
  {
    if (_type == TypeInvalid)
    {
      _type = t;
      switch (_type) {    // Ensure there is a valid value for the type
		case TypeInt:	   _value.asInt = 0;					break;
        case TypeString:   _value.asString = new std::string(); break;
        case TypeDateTime: _value.asTime = new struct tm();     break;
        case TypeArray:    _value.asArray = new ValueArray();   break;
        case TypeStruct:   _value.asStruct = new ValueStruct(); break;
        default:           break;
      }
    }
    else if (_type != t)
	{
		//格式错误，异常情况
	}
  }

  void CSkpRoadXmlValue::assertArray(int size) const
  {
    if (_type != TypeArray)
	{
		//格式错误，异常情况

	 }
    else if (int(_value.asArray->size()) < size)
	  {
		//格式错误，异常情况

	 }
}


  void CSkpRoadXmlValue::assertArray(int size)
  {
    if (_type == TypeInvalid) {
      _type = TypeArray;
      _value.asArray = new ValueArray(size);
    } else if (_type == TypeArray) {
      if (int(_value.asArray->size()) < size)
        _value.asArray->resize(size);
    } else
      {
		//格式错误，异常情况

	  }
  }

  void CSkpRoadXmlValue::assertStruct()
  {
    if (_type == TypeInvalid) {
      _type = TypeStruct;
      _value.asStruct = new ValueStruct();
    } else if (_type != TypeStruct)
      {
		//格式错误，异常情况

	  }
  }


  // Operators
  CSkpRoadXmlValue& CSkpRoadXmlValue::operator=(CSkpRoadXmlValue const& rhs)
  {
    if (this != &rhs)
    {
      invalidate();
      _type = rhs._type;
      switch (_type) {
        case TypeBoolean:  _value.asBool = rhs._value.asBool; break;
        case TypeInt:      _value.asInt = rhs._value.asInt; break;
        case TypeDouble:   _value.asDouble = rhs._value.asDouble; break;
        case TypeDateTime: _value.asTime = new struct tm(*rhs._value.asTime); break;
        case TypeString:   _value.asString = new std::string(*rhs._value.asString); break;
        case TypeArray:    _value.asArray = new ValueArray(*rhs._value.asArray); break;
        case TypeStruct:   _value.asStruct = new ValueStruct(*rhs._value.asStruct); break;
        default:           break;
      }
    }
    return *this;
  }


  // Predicate for tm equality
  static bool tmEq(struct tm const& t1, struct tm const& t2) {
    return t1.tm_sec == t2.tm_sec && t1.tm_min == t2.tm_min &&
            t1.tm_hour == t2.tm_hour && t1.tm_mday == t1.tm_mday &&
            t1.tm_mon == t2.tm_mon && t1.tm_year == t2.tm_year;
  }

  bool CSkpRoadXmlValue::operator==(CSkpRoadXmlValue const& other) const
  {
    if (_type != other._type)
      return false;

    switch (_type) {
      case TypeBoolean:  return ( !_value.asBool && !other._value.asBool) ||
                                ( _value.asBool && other._value.asBool);
      case TypeInt:      return _value.asInt == other._value.asInt;
      case TypeDouble:   return _value.asDouble == other._value.asDouble;
      case TypeDateTime: return tmEq(*_value.asTime, *other._value.asTime);
      case TypeString:   return *_value.asString == *other._value.asString;
      case TypeArray:    return *_value.asArray == *other._value.asArray;

      // The map<>::operator== requires the definition of value< for kcc
      case TypeStruct:   //return *_value.asStruct == *other._value.asStruct;
        {
          if (_value.asStruct->size() != other._value.asStruct->size())
            return false;
          
          ValueStruct::const_iterator it1=_value.asStruct->begin();
          ValueStruct::const_iterator it2=other._value.asStruct->begin();
          while (it1 != _value.asStruct->end()) {
            const CSkpRoadXmlValue& v1 = it1->second;
            const CSkpRoadXmlValue& v2 = it2->second;
            if ( ! (v1 == v2))
              return false;
            it1++;
            it2++;
          }
          return true;
        }
      default: break;
    }
    return true;    // Both invalid values ...
  }

  bool CSkpRoadXmlValue::operator!=(CSkpRoadXmlValue const& other) const
  {
    return !(*this == other);
  }


  // Works for strings, binary data, arrays, and structs.
  int CSkpRoadXmlValue::size() const
  {
    switch (_type) {
      case TypeString: return int(_value.asString->size());
      case TypeArray:  return int(_value.asArray->size());
      case TypeStruct: return int(_value.asStruct->size());
      default: break;
    }
		//格式错误，异常情况
	return 0;
  }

  // Checks for existence of struct member
  bool CSkpRoadXmlValue::hasMember(const std::string& name) const
  {
    return _type == TypeStruct && _value.asStruct->find(name) != _value.asStruct->end();
  }

  // Set the value from xml. The chars at *offset into valueXml 
  // should be the start of a <value> tag. Destroys any existing value.
  bool CSkpRoadXmlValue::fromXml(std::string const& valueXml, int* offset)
  {
    int savedOffset = *offset;

    invalidate();
    if ( ! CSkpRoadXmlUtil::nextTagIs(VALUE_TAG, valueXml, offset))
      return false;       // Not a value, offset not updated

	int afterValueOffset = *offset;
    std::string typeTag = CSkpRoadXmlUtil::getNextTag(valueXml, offset);
    bool result = false;
    if (typeTag == BOOLEAN_TAG)
      result = boolFromXml(valueXml, offset);
    else if (typeTag == I4_TAG || typeTag == INT_TAG)
      result = intFromXml(valueXml, offset);
    else if (typeTag == DOUBLE_TAG)
      result = doubleFromXml(valueXml, offset);
    else if (typeTag.empty() || typeTag == STRING_TAG)
      result = stringFromXml(valueXml, offset);
    else if (typeTag == DATETIME_TAG)
      result = timeFromXml(valueXml, offset);
    else if (typeTag == ARRAY_TAG)
      result = arrayFromXml(valueXml, offset);
    else if (typeTag == STRUCT_TAG)
      result = structFromXml(valueXml, offset);
    // Watch for empty/blank strings with no <string>tag
    else if (typeTag == VALUE_ETAG)
    {
      *offset = afterValueOffset;   // back up & try again
      result = stringFromXml(valueXml, offset);
    }

    if (result)  // Skip over the </value> tag
      CSkpRoadXmlUtil::findTag(VALUE_ETAG, valueXml, offset);
    else        // Unrecognized tag after <value>
      *offset = savedOffset;

    return result;
  }

  // Encode the Value in xml
  std::string CSkpRoadXmlValue::toXml() const
  {
    switch (_type) {
      case TypeBoolean:  return boolToXml();
      case TypeInt:      return intToXml();
      case TypeDouble:   return doubleToXml();
      case TypeString:   return stringToXml();
      case TypeDateTime: return timeToXml();
      case TypeArray:    return arrayToXml();
      case TypeStruct:   return structToXml();
      default: break;
    }
    return std::string();   // Invalid value
  }


  // Boolean
  bool CSkpRoadXmlValue::boolFromXml(std::string const& valueXml, int* offset)
  {
    const char* valueStart = valueXml.c_str() + *offset;
    char* valueEnd;
    long ivalue = strtol(valueStart, &valueEnd, 10);
    if (valueEnd == valueStart || (ivalue != 0 && ivalue != 1))
      return false;

    _type = TypeBoolean;
    _value.asBool = (ivalue == 1);
    *offset += int(valueEnd - valueStart);
    return true;
  }

  std::string CSkpRoadXmlValue::boolToXml() const
  {
    std::string xml = VALUE_TAG;
    xml += BOOLEAN_TAG;
    xml += (_value.asBool ? "1" : "0");
    xml += BOOLEAN_ETAG;
    xml += VALUE_ETAG;
    return xml;
  }

  // Int
  bool CSkpRoadXmlValue::intFromXml(std::string const& valueXml, int* offset)
  {
    const char* valueStart = valueXml.c_str() + *offset;
    char* valueEnd;
    long ivalue = strtol(valueStart, &valueEnd, 10);
    if (valueEnd == valueStart)
      return false;

    _type = TypeInt;
    _value.asInt = int(ivalue);
    *offset += int(valueEnd - valueStart);
    return true;
  }

  std::string CSkpRoadXmlValue::intToXml() const
  {
    char buf[256];
    snprintf(buf, sizeof(buf)-1, "%d", _value.asInt);
    buf[sizeof(buf)-1] = 0;
    std::string xml = VALUE_TAG;
    xml += I4_TAG;
    xml += buf;
    xml += I4_ETAG;
    xml += VALUE_ETAG;
    return xml;
  }

  // Double
  bool CSkpRoadXmlValue::doubleFromXml(std::string const& valueXml, int* offset)
  {
    const char* valueStart = valueXml.c_str() + *offset;
    char* valueEnd;
    double dvalue = strtod(valueStart, &valueEnd);
    if (valueEnd == valueStart)
      return false;

    _type = TypeDouble;
    _value.asDouble = dvalue;
    *offset += int(valueEnd - valueStart);
    return true;
  }

  std::string CSkpRoadXmlValue::doubleToXml() const
  {
    char buf[256];
    snprintf(buf, sizeof(buf)-1, "%f", _value.asDouble);
    buf[sizeof(buf)-1] = 0;

    std::string xml = VALUE_TAG;
    xml += DOUBLE_TAG;
    xml += buf;
    xml += DOUBLE_ETAG;
    xml += VALUE_ETAG;
    return xml;
  }

  // String
  bool CSkpRoadXmlValue::stringFromXml(std::string const& valueXml, int* offset)
  {
    size_t valueEnd = valueXml.find('<', *offset);
    if (valueEnd == std::string::npos)
      return false;     // No end tag;

    _type = TypeString;
    _value.asString = new std::string(CSkpRoadXmlUtil::xmlDecode(valueXml.substr(*offset, valueEnd-*offset)));
    *offset += int(_value.asString->length());
    return true;
  }

  std::string CSkpRoadXmlValue::stringToXml() const
  {
    std::string xml = VALUE_TAG;
    xml += STRING_TAG;// optional
    xml += CSkpRoadXmlUtil::xmlEncode(*_value.asString);
    xml += STRING_ETAG;
    xml += VALUE_ETAG;
    return xml;
  }

  // DateTime (stored as a struct tm)
  bool CSkpRoadXmlValue::timeFromXml(std::string const& valueXml, int* offset)
  {
    size_t valueEnd = valueXml.find('<', *offset);
    if (valueEnd == std::string::npos)
      return false;     // No end tag;

    std::string stime = valueXml.substr(*offset, valueEnd-*offset);

    struct tm t;
    if (sscanf(stime.c_str(),"%4d%2d%2dT%2d:%2d:%2d",&t.tm_year,&t.tm_mon,&t.tm_mday,&t.tm_hour,&t.tm_min,&t.tm_sec) != 6)
      return false;

    t.tm_isdst = -1;
    _type = TypeDateTime;
    _value.asTime = new struct tm(t);
    *offset += int(stime.length());
    return true;
  }

  std::string CSkpRoadXmlValue::timeToXml() const
  {
    struct tm* t = _value.asTime;
    char buf[20];
    snprintf(buf, sizeof(buf)-1, "%4d%02d%02dT%02d:%02d:%02d", 
      t->tm_year,t->tm_mon,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
    buf[sizeof(buf)-1] = 0;

    std::string xml = VALUE_TAG;
    xml += DATETIME_TAG;
    xml += buf;
    xml += DATETIME_ETAG;
    xml += VALUE_ETAG;
    return xml;
  }
  // Array
  bool CSkpRoadXmlValue::arrayFromXml(std::string const& valueXml, int* offset)
  {
    if ( ! CSkpRoadXmlUtil::nextTagIs(DATA_TAG, valueXml, offset))
      return false;

    _type = TypeArray;
    _value.asArray = new ValueArray;
    CSkpRoadXmlValue v;
    while (v.fromXml(valueXml, offset))
      _value.asArray->push_back(v);       // copy...

    // Skip the trailing </data>
    (void) CSkpRoadXmlUtil::nextTagIs(DATA_ETAG, valueXml, offset);
    return true;
  }


  // In general, its preferable to generate the xml of each element of the
  // array as it is needed rather than glomming up one big string.
  std::string CSkpRoadXmlValue::arrayToXml() const
  {
    std::string xml = VALUE_TAG;
    xml += ARRAY_TAG;
    xml += DATA_TAG;

    int s = int(_value.asArray->size());
    for (int i=0; i<s; ++i)
       xml += _value.asArray->at(i).toXml();

    xml += DATA_ETAG;
    xml += ARRAY_ETAG;
    xml += VALUE_ETAG;
    return xml;
  }


  // Struct
  bool CSkpRoadXmlValue::structFromXml(std::string const& valueXml, int* offset)
  {
    _type = TypeStruct;
    _value.asStruct = new ValueStruct;

    while (CSkpRoadXmlUtil::nextTagIs(MEMBER_TAG, valueXml, offset)) {
      // name
      const std::string name = CSkpRoadXmlUtil::parseTag(NAME_TAG, valueXml, offset);
      // value
      CSkpRoadXmlValue val(valueXml, offset);
      if ( ! val.valid()) {
        invalidate();
        return false;
      }
      const std::pair<const std::string, CSkpRoadXmlValue> p(name, val);
      _value.asStruct->insert(p);

      (void) CSkpRoadXmlUtil::nextTagIs(MEMBER_ETAG, valueXml, offset);
    }
    return true;
  }


  // In general, its preferable to generate the xml of each element
  // as it is needed rather than glomming up one big string.
  std::string CSkpRoadXmlValue::structToXml() const
  {
    std::string xml = VALUE_TAG;
    xml += STRUCT_TAG;

    ValueStruct::const_iterator it;
    for (it=_value.asStruct->begin(); it!=_value.asStruct->end(); ++it) {
      xml += MEMBER_TAG;
      xml += NAME_TAG;
      xml += CSkpRoadXmlUtil::xmlEncode(it->first);
      xml += NAME_ETAG;
      xml += it->second.toXml();
      xml += MEMBER_ETAG;
    }

    xml += STRUCT_ETAG;
    xml += VALUE_ETAG;
    return xml;
  }
