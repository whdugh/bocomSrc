// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

// VehicleConfig.cpp: implementation of the CVehicleConfig class.
//
//////////////////////////////////////////////////////////////////////
#include "VehicleConfig.h"


#ifdef _WINDOWS
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#endif

//#define MAX_SIZE 255
//#define _PRINT_

CVehicleConfig::CVehicleConfig()
{
	m_strName = NULL;
}

CVehicleConfig::~CVehicleConfig()
{

}



CVehicleConfig::CVehicleConfig(const char *strName)
{
	if(strName == NULL)
		return;

	m_strName = strName;
}

void CVehicleConfig::SetFilePath(const char *strName)
{
#ifdef _PRINT
	printf("Config path = %s\r\n", strName);
#endif
	if(strName == NULL)
		return;

	m_strName = strName;
}

void CVehicleConfig::WriteFile(const char *strName)
{
	if(strName == NULL)
		return;
	/* Set file name		*/
	m_strName = strName;
	/* Write file			*/
	WriteFile();
}

void CVehicleConfig::WriteFile(CHANNEL_INFO &channel_info)
{
	chProp_index		= channel_info.chProp_index;
	chProp_name			= channel_info.chProp_name;
	chProp_direction	= channel_info.chProp_direction;
	roadRegion			= channel_info.roadRegion;
	chRegion			= channel_info.chRegion;
	stopRegion			= channel_info.stopRegion;
	personRegion		= channel_info.personRegion;
	dropRegion			= channel_info.dropRegion;

	AmountLine			= channel_info.AmountLine;

	RefLine				= channel_info.RefLine;
	TurnRoadLine		= channel_info.TurnRoadLine;
	eliminateRegion		= channel_info.eliminateRegion;
	carnumRegion		= channel_info.carnumRegion;
	calibration			= channel_info.calibration;

    flowbackRegion      = channel_info.FlowBackRegion;
	flowLine            = channel_info.FlowLine;
	flowframegetRegion  = channel_info.FlowFramegetRegion;

	ViolationRegion         = channel_info.ViolationRegion;
	EventRegion             = channel_info.EventRegion;
	TrafficSignalRegion     = channel_info.TrafficSignalRegion;
	StopLine                = channel_info.StopLine;
	StraightLine            = channel_info.StraightLine;
	TurnLeftLine            = channel_info.TurnLeftLine;
	TurnRightLine           = channel_info.TurnRightLine;

	StabBackRegion          = channel_info.StabBackRegion;
	SynchLeftRegion         = channel_info.SynchLeftRegion;
	SynchRightRegion        = channel_info.SynchRightRegion;

	CardnumberRegion        = channel_info.CardnumberRegion;
	LoopRegion              = channel_info.LoopRegion;

	WriteFile();
}


void CVehicleConfig::WriteFile()
{
#ifdef _PRINT
		printf("Before Write file :%s\r\n\n\n",m_strName);
#endif
	/* If file path is invalid, do nothing */
	if(m_strName == NULL) return;

#ifdef _PRINT
		printf("Write file :%s\r\n\n\n",m_strName);
#endif

	/* outfile for writing		*/
	ofstream   outfile(m_strName);

	/* Save memory text to disk	*/

	/* Save INDEX property of channel	*/
	outfile<<chProp_index.strName<<"\t"<<chProp_index.value.nValue<<endl<<endl;

	/* Save NAME property of channel	*/
	outfile<<chProp_name.strName<<"\t"<<chProp_name.value.strValue<<endl<<endl;

	/* Save DIRECTION property of channel	*/
	outfile<<chProp_direction.strName<<"\t"<<chProp_direction.value.nValue<<' '<<chProp_direction.point.x<<' '<<chProp_direction.point.y<<endl<<endl;

	/* Save REGION property of road		*/
	outfile<<roadRegion.chProperty.strName<<'\t'<<roadRegion.chProperty.value.nValue<<"\t";
	std::list<CPoint32f>::iterator it_begin = roadRegion.listPT.begin();
	std::list<CPoint32f>::iterator it_end = roadRegion.listPT.end();
	for(; it_begin != it_end; it_begin++)
	{
		outfile<<it_begin->x<<' ';
		outfile<<it_begin->y<<' ';
		outfile<<' ';
	}
	outfile<<endl<<endl;

	/* Save REGION property of channel		*/
	outfile<<chRegion.chProperty.strName<<'\t'<<chRegion.chProperty.value.nValue<<"\t";
	it_begin = chRegion.listPT.begin();
	it_end = chRegion.listPT.end();
	for(; it_begin != it_end; it_begin++)
	{
		outfile<<it_begin->x<<' ';
		outfile<<it_begin->y<<' ';
		outfile<<' ';
	}
	outfile<<endl<<endl;

	/* Save STOP region	*/
	outfile<<stopRegion.chProperty.strName<<'\t'<<stopRegion.chProperty.value.nValue<<'\t';

	std::list<REGION_PROPERTY>::iterator it_b = stopRegion.listRegionProp.begin();
	std::list<REGION_PROPERTY>::iterator it_e = stopRegion.listRegionProp.end();
	for(int i = 0; i < stopRegion.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;

	/* Save PERSON region	*/
	outfile<<personRegion.chProperty.strName<<'\t'<<personRegion.chProperty.value.nValue<<'\t';

	it_b = personRegion.listRegionProp.begin();
	it_e = personRegion.listRegionProp.end();
	for(int i = 0; i < personRegion.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;

	/* Save DROP region	*/
	outfile<<dropRegion.chProperty.strName<<'\t'<<dropRegion.chProperty.value.nValue<<'\t';

	it_b = dropRegion.listRegionProp.begin();
	it_e = dropRegion.listRegionProp.end();
	for(int i = 0; i < dropRegion.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;

	/* Save TRAFFIC AMOUNTLINE region	*/
	outfile<<AmountLine.chProperty.strName<<'\t'<<AmountLine.chProperty.value.nValue<<'\t';

	it_b = AmountLine.listRegionProp.begin();
	it_e = AmountLine.listRegionProp.end();
	for(int i = 0; i < AmountLine.chProperty.value.nValue; i++)
	{
		//	outfile<<it_b->nFirstValue<<"   ";
		//	outfile<<it_b->nSecondValue<<"   ";
		outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;


	/* Save REFERENCE FLOWLINE region	*/
	outfile<<RefLine.chProperty.strName<<'\t'<<RefLine.chProperty.value.nValue<<'\t';

	it_b = RefLine.listRegionProp.begin();
	it_e = RefLine.listRegionProp.end();
	for(int i = 0; i < RefLine.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;

	/* Save TURNROADLINE region	*/
	outfile<<TurnRoadLine.chProperty.strName<<'\t'<<TurnRoadLine.chProperty.value.nValue<<'\t';

	it_b = TurnRoadLine.listRegionProp.begin();
	it_e = TurnRoadLine.listRegionProp.end();
	for(int i = 0; i < TurnRoadLine.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;

	/* Save CALIBRATION region	*/
	outfile<<calibration.region.chProperty.strName<<'\t'<<calibration.region.chProperty.value.nValue<<"\t";

	it_begin = calibration.region.listPT.begin();
	it_end = calibration.region.listPT.end();

    int k = 0;
	for(; it_begin != it_end; it_begin++)
	{
		//image cor
		outfile<<it_begin->x<<' ';
		outfile<<it_begin->y<<' ';
        //world cor
		switch (k)
		{
			case 0:
                outfile<<0<<' '<<0<<' '<<0;
				break;
			case 1:
				outfile<<calibration.length<<' '<<0<<' '<<0;
				break;
			case 2:
				outfile<<calibration.length<<' '<<calibration.width<<' '<<0;
				break;
			case 3:
				outfile<<0<<' '<<calibration.width<<' '<<0;
				break;
		}

		outfile<<endl;
		k++;
	}

	it_begin = calibration.listPT.begin();
	it_end = calibration.listPT.end();

	std::list<CPoint32f>::iterator it_32fb = calibration.list32fPT.begin();
	std::list<CPoint32f>::iterator it_32fe = calibration.list32fPT.end();
	while(it_begin!=it_end&&it_32fb!=it_32fe)
	{
		//image cor
        outfile<<it_begin->x<<' ';
		outfile<<it_begin->y<<' ';

		//world cor
		outfile<<it_32fb->x<<' ';
		outfile<<it_32fb->y<<' ';
		outfile<<it_32fb->z<<' ';

		outfile<<endl;
        it_32fb++;
		it_begin++;
	}

	outfile<<calibration.cameraHeight<<endl;
	outfile<<endl;

/*	outfile<<endl;
	outfile<<calibration.length<<' '<<calibration.width<<endl;
	outfile<<calibration.cameraHeight<<' ';
	outfile<<calibration.cameraDistance<<' ';
	outfile<<calibration.cameraOrientation<<endl<<endl;*/
	/* Save SKIP region	*/
	outfile<<eliminateRegion.chProperty.strName<<'\t'<<eliminateRegion.chProperty.value.nValue<<'\t';

	it_b = eliminateRegion.listRegionProp.begin();
	it_e = eliminateRegion.listRegionProp.end();
	for(int i = 0; i < eliminateRegion.chProperty.value.nValue; i++)
	{
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;

	/* Save CARNUM region	*/
	outfile<<carnumRegion.chProperty.strName<<'\t'<<carnumRegion.chProperty.value.nValue<<'\t';

	it_b = carnumRegion.listRegionProp.begin();
	it_e = carnumRegion.listRegionProp.end();
	for(int i = 0; i < carnumRegion.chProperty.value.nValue; i++)
	{
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;

///////////////////////////////////////////////////////////----FLOW--begin

    /* Save FLOWBACK region	*/
	outfile<<flowbackRegion.chProperty.strName<<'\t'<<flowbackRegion.chProperty.value.nValue<<'\t';

	it_b = flowbackRegion.listRegionProp.begin();
	it_e = flowbackRegion.listRegionProp.end();
	for(int i = 0; i < flowbackRegion.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;

	/* Save FLOWLINE region	*/
	outfile<<flowLine.chProperty.strName<<'\t'<<flowLine.chProperty.value.nValue<<'\t';

	it_b = flowLine.listRegionProp.begin();
	it_e = flowLine.listRegionProp.end();
	for(int i = 0; i < flowLine.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;

	/* Save FLOWFREAMEGET region	*/
	outfile<<flowframegetRegion.chProperty.strName<<'\t'<<flowframegetRegion.chProperty.value.nValue<<'\t';

	it_b = flowframegetRegion.listRegionProp.begin();
	it_e = flowframegetRegion.listRegionProp.end();
	for(int i = 0; i < flowframegetRegion.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;
///////////////////////////////////////////////////////////----FLOW--end
/////////////////////////////////////////////闯红灯--begin

/* Save ViolationArea region	*/
	outfile<<ViolationRegion.chProperty.strName<<'\t'<<ViolationRegion.chProperty.value.nValue<<'\t';

	it_b = ViolationRegion.listRegionProp.begin();
	it_e = ViolationRegion.listRegionProp.end();
	for(int i = 0; i < ViolationRegion.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;
/* Save Event region	*/
	outfile<<EventRegion.chProperty.strName<<'\t'<<EventRegion.chProperty.value.nValue<<'\t';

	it_b = EventRegion.listRegionProp.begin();
	it_e = EventRegion.listRegionProp.end();
	for(int i = 0; i < EventRegion.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;
/* Save TrafficSignalArea region	*/
	outfile<<TrafficSignalRegion.chProperty.strName<<'\t'<<TrafficSignalRegion.chProperty.value.nValue<<'\t';

	it_b = TrafficSignalRegion.listRegionProp.begin();
	it_e = TrafficSignalRegion.listRegionProp.end();
	for(int i = 0; i < TrafficSignalRegion.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;
/* Save StopLine */
	outfile<<StopLine.chProperty.strName<<'\t'<<StopLine.chProperty.value.nValue<<'\t';

	it_b = StopLine.listRegionProp.begin();
	it_e = StopLine.listRegionProp.end();
	for(int i = 0; i < StopLine.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;
/* Save StraightLinea */
	outfile<<StraightLine.chProperty.strName<<'\t'<<StraightLine.chProperty.value.nValue<<'\t';

	it_b = StraightLine.listRegionProp.begin();
	it_e = StraightLine.listRegionProp.end();
	for(int i = 0; i < StraightLine.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;
/* Save TurnLeftLine */
	outfile<<TurnLeftLine.chProperty.strName<<'\t'<<TurnLeftLine.chProperty.value.nValue<<'\t';

	it_b = TurnLeftLine.listRegionProp.begin();
	it_e = TurnLeftLine.listRegionProp.end();
	for(int i = 0; i < TurnLeftLine.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;
/* Save TurnRightLine */
	outfile<<TurnRightLine.chProperty.strName<<'\t'<<TurnRightLine.chProperty.value.nValue<<'\t';

	it_b = TurnRightLine.listRegionProp.begin();
	it_e = TurnRightLine.listRegionProp.end();
	for(int i = 0; i < TurnRightLine.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;
/////////////////////////////////////////////闯红灯--end

/* Save STAB_BACK region	*/
	outfile<<StabBackRegion.chProperty.strName<<'\t'<<StabBackRegion.chProperty.value.nValue<<'\t';

	it_b = StabBackRegion.listRegionProp.begin();
	it_e = StabBackRegion.listRegionProp.end();
	for(int i = 0; i < StabBackRegion.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;
/* Save SYNCH_LEFT_REGION region	*/
	outfile<<SynchLeftRegion.chProperty.strName<<'\t'<<SynchLeftRegion.chProperty.value.nValue<<'\t';

	it_b = SynchLeftRegion.listRegionProp.begin();
	it_e = SynchLeftRegion.listRegionProp.end();
	for(int i = 0; i < SynchLeftRegion.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;
/* Save SYNCH_RIGHT_REGION region	*/
	outfile<<SynchRightRegion.chProperty.strName<<'\t'<<SynchRightRegion.chProperty.value.nValue<<'\t';

	it_b = SynchRightRegion.listRegionProp.begin();
	it_e = SynchRightRegion.listRegionProp.end();
	for(int i = 0; i < SynchRightRegion.chProperty.value.nValue; i++)
	{
	//	outfile<<it_b->nFirstValue<<"   ";
	//	outfile<<it_b->nSecondValue<<"   ";
	    outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
	outfile<<endl;

/* Save CARDNUMBER region	*/
	outfile<<CardnumberRegion.chProperty.strName<<'\t'<<CardnumberRegion.chProperty.value.nValue<<'\t';

	it_b = CardnumberRegion.listRegionProp.begin();
	it_e = CardnumberRegion.listRegionProp.end();
	for(int i = 0; i < CardnumberRegion.chProperty.value.nValue; i++)
	{
		//	outfile<<it_b->nFirstValue<<"   ";
		//	outfile<<it_b->nSecondValue<<"   ";
		outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
    outfile<<endl;

/* Save LooPLine region	*/
	outfile<<LoopRegion.chProperty.strName<<'\t'<<LoopRegion.chProperty.value.nValue<<'\t';

	it_b = LoopRegion.listRegionProp.begin();
	it_e = LoopRegion.listRegionProp.end();
	for(int i = 0; i < LoopRegion.chProperty.value.nValue; i++)
	{
		//	outfile<<it_b->nFirstValue<<"   ";
		//	outfile<<it_b->nSecondValue<<"   ";
		outfile<<it_b->nValue<<"   ";
		Point32fList::iterator item_b = it_b->listPt.begin();
		for(int j = 0; j < it_b->nValue; j++)
		{
			outfile<<item_b->x<<"   ";
			outfile<<item_b->y<<"   ";
			item_b++;
		}
		it_b++;
		outfile<<endl;
	}
//outfile<<endl;

	/* Close the file	*/
	outfile.close();

}

void CVehicleConfig::SetChannelIndex(int nIndex)
{
	chProp_index.value.nValue = nIndex;
}

void CVehicleConfig::SetChannelName(const char *strName)
{
	strcpy(chProp_name.value.strValue, strName);
}

void CVehicleConfig::SetChannelDirection(int nDirection)
{
	chProp_direction.value.nValue = nDirection;
}

void CVehicleConfig::SetRoadRegion(CHANNEL_REGION *region)
{
	roadRegion.listPT.clear();
	roadRegion = *region;
}

void CVehicleConfig::SetChannelRegion(CHANNEL_REGION *region)
{
	chRegion.listPT.clear();
	chRegion = *region;
}

void CVehicleConfig::SetStopRegion(COMMON_REGION *pStopRegion)
{
	stopRegion.listRegionProp.clear();
	stopRegion = *pStopRegion;
}

void CVehicleConfig::SetPersonRegion(COMMON_REGION *pPersonRegion)
{
	personRegion.listRegionProp.clear();
	personRegion = *pPersonRegion;
}

void CVehicleConfig::SetDropRegion(COMMON_REGION *pDropRegion)
{
	dropRegion.listRegionProp.clear();
	dropRegion = *pDropRegion;
}

void CVehicleConfig::SetAmountLine(COMMON_REGION *pAmountLine)
{
	AmountLine.listRegionProp.clear();
	AmountLine = *pAmountLine;
}

void CVehicleConfig::SetRefLine(COMMON_REGION *pRefLine)
{
	RefLine.listRegionProp.clear();
	RefLine = *pRefLine;
}

void CVehicleConfig::SetTurnRoadLine(COMMON_REGION *pTurnRoadLine)
{
    TurnRoadLine.listRegionProp.clear();
	TurnRoadLine = *pTurnRoadLine;
}

void CVehicleConfig::SetCalibration(CALIBRATION *pCalibration)
{
	calibration.region.listPT.clear();
	calibration = *pCalibration;
}

void CVehicleConfig::SetEliminateRegion(COMMON_REGION *pEliminateRegion)
{
	eliminateRegion.listRegionProp.clear();
	eliminateRegion = *pEliminateRegion;
}

void CVehicleConfig::SetCarnumRegion(COMMON_REGION *pCarnumRegion)
{
	carnumRegion.listRegionProp.clear();
	carnumRegion = *pCarnumRegion;
}

/* Set FlowBackArea Region  	*/
void CVehicleConfig::SetFlowBackRegion(COMMON_REGION *pFlowBackRegion)
{
    flowbackRegion.listRegionProp.clear();
    flowbackRegion = *pFlowBackRegion;
}

/* Set FlowLine                 */
void CVehicleConfig::SetFlowLine(COMMON_REGION *pFlowLine)
{
    flowLine.listRegionProp.clear();
    flowLine = *pFlowLine;
}

/* Set FlowFramegetArea Region  */
void CVehicleConfig::SetFlowFramegetRegion(COMMON_REGION *pFlowFramegetRegion)
{
    flowframegetRegion.listRegionProp.clear();
    flowframegetRegion = *pFlowFramegetRegion;
}

/* Set ViolationArea Region  	*/
void CVehicleConfig::SetViolationRegion(COMMON_REGION *pViolationRegion)
{
    ViolationRegion.listRegionProp.clear();
    ViolationRegion = *pViolationRegion;
}
/* Set EventArea Region  	    */
void CVehicleConfig::SetEventRegion(COMMON_REGION *pEventRegion)
{
    EventRegion.listRegionProp.clear();
    EventRegion = *pEventRegion;
}
/* Set TrafficSignalArea Region */
void CVehicleConfig::SetTrafficSignalRegion(COMMON_REGION *pTrafficSignalRegion)
{
    TrafficSignalRegion.listRegionProp.clear();
    TrafficSignalRegion = *pTrafficSignalRegion;
}
/* Set StopLine  	    */
void CVehicleConfig::SetStopLine(COMMON_REGION *pStopLine)
{
    StopLine.listRegionProp.clear();
    StopLine = *pStopLine;
}
/* Set StraightLine 	*/
void CVehicleConfig::SetStraightLine(COMMON_REGION *pStraightLine)
{
    StraightLine.listRegionProp.clear();
    StraightLine = *pStraightLine;
}
/* Set TurnLeftLine */
void CVehicleConfig::SetTurnLeftLine(COMMON_REGION *pTurnLeftLine)
{
    TurnLeftLine.listRegionProp.clear();
    TurnLeftLine = *pTurnLeftLine;
}
/* Set TurnRightLine */
void CVehicleConfig::SetTurnRightLine(COMMON_REGION *pTurnRightLine)
{
    TurnRightLine.listRegionProp.clear();
    TurnRightLine = *pTurnRightLine;
}

/* Set StabBack Region  	*/
void CVehicleConfig::SetStabBackRegion(COMMON_REGION *pStabBackRegion)
{
    StabBackRegion.listRegionProp.clear();
    StabBackRegion = *pStabBackRegion;
}

/* Set SynchLeft Region  	*/
void CVehicleConfig::SetSynchLeftRegion(COMMON_REGION *pSynchLeftRegion)
{
    SynchLeftRegion.listRegionProp.clear();
    SynchLeftRegion = *pSynchLeftRegion;
}

/* Set SynchRight Region  	*/
void CVehicleConfig::SetSynchRightRegion(COMMON_REGION *pSynchRightRegion)
{
    SynchRightRegion.listRegionProp.clear();
    SynchRightRegion = *pSynchRightRegion;
}

/* Set Cardnumber Region  */
void CVehicleConfig::SetCardnumberRegion(COMMON_REGION *pCardnumberRegion)
{
    CardnumberRegion.listRegionProp.clear();
    CardnumberRegion = *pCardnumberRegion;
}

/* Set Loop Region  */
void CVehicleConfig::SetLoopRegion(COMMON_REGION *pLoopRegion)
{
    LoopRegion.listRegionProp.clear();
    LoopRegion = *pLoopRegion;
}
