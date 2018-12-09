
#ifndef BRAND_SUSECTION_CODE
#define BRAND_SUSECTION_CODE
#include "global.h"

class CBrandSusection
{
private:
	UINT32 GetCorrectBasePoint(UINT32 &uCarBrand);
	UINT32 GetCarBasePoint(UINT32 uMin,UINT32 uMax,UINT32 uCarBrand);
public:
	void GetCarLabelAndChildSub(UINT32 &uCarBrand,UINT32 &uDetailCarBrand);
	string GetCarLabelText(UINT32 uCarLabel);
	UINT32 GetOldBrandFromDetail(UINT32 uDetalBrand);
};

#endif