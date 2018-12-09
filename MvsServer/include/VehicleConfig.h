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

#ifndef _VEHICLE_H
#define _VEHICLE_H

#ifdef _WINDOWS
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#endif

/*#include <string.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <list>*/
#include "global.h"

using namespace std;


#define NAME_SIZE 32

/* Point struct */
/*typedef struct _POINT
{
	double x;
	double y;
	double z;
	_POINT()
	{
		x = 0;
		y = 0;
		z = 0;
	}

}CPoint32f;

typedef std::list<CPoint32f> Point32fList;*/

/* Channel property struct */
typedef struct _CHANNEL_PROPERTY
{
	char strName[NAME_SIZE];
	struct _VALUE
	{
		int		nValue;
		char	strValue[NAME_SIZE];
	}value;

	CPoint32f point;
	CPoint32f ptBegin;
	CPoint32f ptEnd;
 /*	_CHANNEL_PROPERTY operator=(_CHANNEL_PROPERTY& property)
 	{
		strcpy(str
 		value.nValue = property.value.nValue;
 		strcpy(value.strValue,property.value.strValue);
 		return property;
 	}*/

}CHANNEL_PROPERTY;

/* Channel region struct		*/
/* Include the point pair POINT	*/
typedef struct _CHANNEL_REGION
{
	CHANNEL_PROPERTY chProperty;
	Point32fList listPT;

}CHANNEL_REGION;

typedef struct _REGION_PROPERTY
{
 // int nFirstValue;							/* Common region firstValue */
//	int nSecondValue;							/* Common region property	*/
    int nValue;							        /* Common region property	*/
	Point32fList listPt;							/* Point list*/

	Point32fList directionListPt;    //direction point List 

	void Clear()
	{
	//	nFirstValue = 0;
	//	nSecondValue = 0;
		nValue = 0;
		listPt.clear();
		directionListPt.clear();//add by Gaoxiang
	}
}REGION_PROPERTY;

/* Common region struct			*/
/* Include STOP_REGION , PER_REGION, DROP_REGION, AMOUNT_LINE,REF_LINE	*/
typedef struct _COMMON_REGION
{
	CHANNEL_PROPERTY chProperty;				/* Common region property		*/
	std::list<REGION_PROPERTY> listRegionProp;	/* Common region property list	*/

}COMMON_REGION;

typedef struct _CALIBRATION
{
	double length;									/* Road height			*/
	double width;									/* Road width			*/
	double cameraHeight;							/* Camera height		*/
	//double cameraDistance;							/* Camera distance		*/
	//double cameraOrientation;						/* Camera orientation		*/
	CHANNEL_REGION region;						/* Calibration region	*/
	Point32fList listPT;                           /* CalibrationPoint */
	Point32fList list32fPT;                        /* Calibration32fPoint */

}CALIBRATION;

/* Channel information*/
/* Includes channel property, channel region and other regions */
typedef struct _CHANNEL_INFO
{
	CHANNEL_PROPERTY	chProp_index;			/* Index of the channel property		*/
	CHANNEL_PROPERTY	chProp_name;			/* Name of the channel property			*/
	CHANNEL_PROPERTY	chProp_direction;		/* Direction of the channel property	*/

    CHANNEL_REGION      roadRegion;              /* Road region*/
	CHANNEL_REGION		chRegion;				/* Channel region		*/

	COMMON_REGION		stopRegion;				/* Stop region			*/
	COMMON_REGION		personRegion;			/* Person region		*/
	COMMON_REGION		dropRegion;				/* Drop region			*/

	COMMON_REGION		AmountLine;				/* Traffic amount line	*/

	COMMON_REGION		RefLine;				/* Reference line		*/
	COMMON_REGION		TurnRoadLine;			/* TurnRoad Line		*/
	COMMON_REGION		eliminateRegion;		/* eliminate Region     */
	COMMON_REGION		carnumRegion;			/* carnum Region        */

	CALIBRATION			calibration;			/* Calibration			*/

    COMMON_REGION      FlowBackRegion;          /* FlowBackArea region*/
	COMMON_REGION      FlowLine;                /* Flow line */
	COMMON_REGION      FlowFramegetRegion;      /* FlowFramegetArea region*/

	COMMON_REGION     ViolationRegion;          /* ViolationRegionArea region*/
	COMMON_REGION     EventRegion;              /* EventRegionArea region*/
	COMMON_REGION     TrafficSignalRegion;      /* TrafficSignalArea region*/
	COMMON_REGION     StopLine;           /* StopLine line*/
	COMMON_REGION     StraightLine;       /* StraightLine line*/
	COMMON_REGION     TurnLeftLine;       /* TurnLeftLine line*/
	COMMON_REGION     TurnRightLine;      /* TurnRightLine line*/
	COMMON_REGION     YellowLine;      /* YellowLine line*/

    COMMON_REGION     LineStop;      /* LineStop */
	COMMON_REGION     RedLightRegion;      /* RedLightRegion*/
	COMMON_REGION     GreenLightRegion;      /* GreenLightRegion*/
	COMMON_REGION     LeftLightRegion;      /* LeftLightRegion*/
	COMMON_REGION     RightLightRegion;      /* RightLightRegion*/
	COMMON_REGION     StraightLightRegion;      /* StraightLightRegion*/
	COMMON_REGION     TurnAroundLightRegion;      /* TurnAroundLightRegion*/
	COMMON_REGION     LeftRedLightRegion;      /* LeftRedLightRegion*/
	COMMON_REGION     LeftGreenLightRegion;      /* LeftGreenLightRegion*/
	COMMON_REGION     RightRedLightRegion;      /* RightRedLightRegion*/
	COMMON_REGION     RightGreenLightRegion;      /* RightGreenLightRegion*/
	COMMON_REGION     StraightRedLightRegion;      /* StraightRedLightRegion*/
	COMMON_REGION     StraightGreenLightRegion;      /* StraightGreenLightRegion*/
    COMMON_REGION     TurnAroundRedLightRegion;      /* TurnAroundRedLightRegion*/
	COMMON_REGION     TurnAroundGreenLightRegion;      /* TurnAroundGreenLightRegion*/
	COMMON_REGION     LineStraight;      /* LineStraight*/

	COMMON_REGION     StabBackRegion;      /* StabBack Region*/
	COMMON_REGION     SynchLeftRegion;      /* Synch RegionLeft*/
	COMMON_REGION     SynchRightRegion;      /* Synch RegionLeft*/

	COMMON_REGION     CardnumberRegion;      /* Cardnumber Region*/
	COMMON_REGION     LoopRegion;           /* LoopLine Region */

	COMMON_REGION     BargeInRegion;           /* BargeInRegion Region */
	COMMON_REGION     BeyondMarkRegion;           /* BeyondMarkRegion Region */
	COMMON_REGION     RadarRegion;           /* RadarRegion Region */
	COMMON_REGION     VirtualLoopRegion;        /* VirtualLoopRegion */
	COMMON_REGION     RemotePersonRegion;        /* RemotePersonRegion */
	COMMON_REGION     LocalPersonRegion;        /* LocalPersonRegion */
	COMMON_REGION	  DensityRegion;	/*DensityRegion*/
	COMMON_REGION	  GetPhotoRegion;	/*抓图区域*/
	COMMON_REGION     WhiteLine;      /* WhiteLine line*/
	COMMON_REGION     LeadStreamLine;      /* LeadStreamLine*/

	COMMON_REGION     HoldForeLineFirst;      /* HoldForeLineFirst*/
	COMMON_REGION     HoldForeLineSecond;      /* HoldForeLineSecond*/
	COMMON_REGION     HoldStopLineFirst;      /* HoldStopLineFirst*/
	COMMON_REGION     HoldStopLineSecond;      /* HoldStopLineSecond*/

	COMMON_REGION	 YelGridRgn;		/* Yellow grid Region */

	COMMON_REGION	 ViolationFirstLine;		/* ViolationFirstLine */
	COMMON_REGION	 ViolationSecondLine;		/* ViolationSecondLine */
	COMMON_REGION	 RightFirstLine;		/* RightFirstLine */
	COMMON_REGION	 LeftFirstLine;		/* LeftFirstLine */
	COMMON_REGION	 ForeFirstLine;		/* ForeFirstLine */

}CHANNEL_INFO;
typedef std::list<CHANNEL_INFO> LIST_CHANNEL_INFO;

class CVehicleConfig
{
public:

	void WriteFile();										/* Write the config file	*/
	void WriteFile(const char *strName);					/* Write the config file	*/
	void WriteFile(CHANNEL_INFO &channel_info);				/* Write the config file	*/
	void SetFilePath(const char *strName);					/* Set config path			*/

	CVehicleConfig(const char *strName);
	CVehicleConfig();
	virtual ~CVehicleConfig();


private:
	void SetChannelIndex(int nIndex);						/* Set channel index		*/
	void SetChannelName(const char* strName);				/* Set channel name			*/
	void SetChannelDirection(int nDirection);				/* Set channel direction	*/

    void SetRoadRegion(CHANNEL_REGION *region);			    /* Set road region		    */
	void SetChannelRegion(CHANNEL_REGION *region);			/* Set channel region		*/

	void SetStopRegion(COMMON_REGION *pStopRegion);			/* Set stop region			*/
	void SetPersonRegion(COMMON_REGION *pPersonRegion);		/* Set person region		*/
	void SetDropRegion(COMMON_REGION *pDropRegion);			/* Set drop region			*/

	void SetAmountLine(COMMON_REGION *pAmountLine);				/* Set traffic Amount line	*/

	void SetRefLine(COMMON_REGION *pRefLine);				/* Set reference line		*/
	void SetTurnRoadLine(COMMON_REGION *pTurnRoadLine);		/* Set turnroad line		*/
	void SetEliminateRegion(COMMON_REGION *pEliminateRegion);/* eliminate Region		*/
	void SetCarnumRegion(COMMON_REGION *pCarnumRegion);		 /* carnum Region		*/

	void SetCalibration(CALIBRATION *pCalibration);			/* Set calibration			*/

	void SetFlowBackRegion(COMMON_REGION *pFlowBackRegion);			        /* Set FlowBackArea Region  	*/
	void SetFlowLine(COMMON_REGION *pFlowLine);			                    /* Set FlowLine           	    */
	void SetFlowFramegetRegion(COMMON_REGION *pFlowFramegetRegion);			/* Set FlowFramegetArea Region  */

	void SetViolationRegion(COMMON_REGION *pViolationRegion);			/* Set ViolationArea Region  	*/
	void SetEventRegion(COMMON_REGION *pEventRegion);			        /* Set EventArea Region  	    */
	void SetTrafficSignalRegion(COMMON_REGION *pTrafficSignalRegion);	/* Set TrafficSignalArea Region */
	void SetStopLine(COMMON_REGION *pStopLineRegion);			    /* Set StopLine Region  	    */
	void SetStraightLine(COMMON_REGION *pStraightLine);		/* Set StraightLine Region  	*/
	void SetTurnLeftLine(COMMON_REGION *pTurnLeftLine);		/* Set TurnLeftLine Region  	*/
	void SetTurnRightLine(COMMON_REGION *pTurnRightLine);	/* Set TurnRightLine Region  	*/

	void SetStabBackRegion(COMMON_REGION *pStabBackRegion);	/* Set StabBack Region  	*/
	void SetSynchLeftRegion(COMMON_REGION *pSynchLeftRegion);	/* Set SynchLeft Region  	*/
	void SetSynchRightRegion(COMMON_REGION *pSynchRightRegion);	/* Set SynchRight Region  	*/

	void SetCardnumberRegion(COMMON_REGION *pCardnumberRegion);	/* Set Cardnumber Region  	*/
	void SetLoopRegion(COMMON_REGION *pLoopRegion);	/* Set LoopLine Region  	*/

private:


	CHANNEL_PROPERTY	chProp_index;			/* Index of the channel property		*/
	CHANNEL_PROPERTY	chProp_name;			/* Name of the channel property			*/
	CHANNEL_PROPERTY	chProp_direction;		/* Direction of the channel property	*/

    CHANNEL_REGION      roadRegion;              /* Road region         */
	CHANNEL_REGION		chRegion;				/*  Channel region		*/

	COMMON_REGION		stopRegion;				/* Stop region			*/
	COMMON_REGION		personRegion;			/* Person region		*/
	COMMON_REGION		dropRegion;				/* Drop region			*/

    COMMON_REGION		AmountLine;				/* Traffic amount line	*/

	COMMON_REGION		RefLine;				/* Reference line		*/
	COMMON_REGION		TurnRoadLine;			/* TurnRoad Line		*/
	COMMON_REGION		eliminateRegion;		/* eliminate Region     */
	COMMON_REGION		carnumRegion;		    /* carnum Region     */

	CALIBRATION			calibration;			/* Calibration			*/

	COMMON_REGION      flowbackRegion;          /* FlowBackArea region*/
	COMMON_REGION      flowLine;                /* FlowLine         */
	COMMON_REGION      flowframegetRegion;      /* FlowFramegetArea region*/

	COMMON_REGION      ViolationRegion;          /* ViolationArea region*/
	COMMON_REGION      EventRegion;          /* EventArea region*/
	COMMON_REGION      TrafficSignalRegion;          /* TrafficSignalArea region*/

	COMMON_REGION      StopLine;          /* StopLine region*/
	COMMON_REGION      StraightLine;          /* StraightLine region*/
	COMMON_REGION      TurnLeftLine;          /* TurnLeftLine region*/
	COMMON_REGION      TurnRightLine;          /* TurnRightLineregion*/

	COMMON_REGION      StabBackRegion;          /* StabBack region*/
	COMMON_REGION      SynchLeftRegion;          /* SynchLeft region*/
	COMMON_REGION      SynchRightRegion;          /* SynchRight region*/

	COMMON_REGION      CardnumberRegion;          /* Cardnumber region*/
	COMMON_REGION      LoopRegion;              /* LoopLine region */

	const char			*m_strName;				/* The name of config file	*/
};

#endif
