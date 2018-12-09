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

#ifndef SKP_ROAD_XMLUTIL_H
#define SKP_ROAD_XMLUTIL_H

#include "global.h"
/******************************************************************************/
//	描述:智能交通检测系统xml模块，解析xml格式
/******************************************************************************/

  class CSkpRoadXmlUtil {
  public:
    // hokey xml parsing
    //! Returns contents between <tag> and </tag>, updates offset to char after </tag>
    static std::string parseTag(const char* tag, std::string const& xml, int* offset);

    //! Returns true if the tag is found and updates offset to the char after the tag
    static bool findTag(const char* tag, std::string const& xml, int* offset);

    //! Returns the next tag and updates offset to the char after the tag, or empty string
    //! if the next non-whitespace character is not '<'
    static std::string getNextTag(std::string const& xml, int* offset);

    //! Returns true if the tag is found at the specified offset (modulo any whitespace)
    //! and updates offset to the char after the tag
    static bool nextTagIs(const char* tag, std::string const& xml, int* offset);

    //! Convert raw text to encoded xml.
    static std::string xmlEncode(const std::string& raw);

    //! Convert encoded xml to raw text
    static std::string xmlDecode(const std::string& encoded);


    //! Dump messages somewhere
    static void log(int level, const char* fmt, ...);

    //! Dump error messages somewhere
    static void error(const char* fmt, ...);

};

#endif
