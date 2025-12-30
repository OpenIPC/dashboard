// IntelligentTrafficCamera.h : main header file for the IntelligentTrafficCamera application
//

#if !defined(AFX_REALLOADPICTURE_H__D79C176B_B614_478F_A246_6DACDBE3869F__INCLUDED_)
#define AFX_REALLOADPICTURE_H__D79C176B_B614_478F_A246_6DACDBE3869F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "map"
#include <vector>
#include <string>
#include "Utility.h"

#define CFG_SECTION_NAME			"INFO"
#define	CFG_KEY_NAME_ALARM_TYPE		"alarm_type"
#define	CFG_KEY_NAME_ALARM_INFO		"alarm_info"
#define	PC_DATA_STORE_FOLDER		"PCStore\\"
#define PC_DATA_STORE_FILE			"cfg.ini"
#define CFG_CFG_FILE_NAME			"Config.ini"
#define CFG_SECTION_CFG				"CFG"
#define CFG_KEY_DATA_PATH			"DATA_PATH"
#define MAX_GUID_LEN 55

#define GET_STRUCT_SIZE			0x01
#define GET_EVENT_CONTENT		0x02
#define GET_DEVICE_ADDRESS		0x04
#define GET_PICTURE_RECT		0x08
#define GET_PLATE_PICTURE_INFO	0x10

const COLORREF RESERVED_COLOR = RGB(255, 255, 255);

typedef struct __COL_DES
{
	std::string strColTitle; // title of column
	int			nColWidth;   // width of column
}ColDes;


struct EVENT_INFO_DISPLAY
{
	std::string strCountNum;			// count num
	std::string strTime;				// time
	std::string strEventType;			// event type

	std::string strFileIndex;			// file index
	std::string strFileCount;			// file count
	std::string strGroupId;				// group id

	std::string strPlateNumber;			// plate text 
	std::string strPlateColor;			// plate color
	std::string strPlateType;			// plate type

	std::string	strVehicleType;			// vehicle type
	std::string strVehicleColor;		// vehicle color
	std::string strVehicleSize;			// vehicle size

	std::string strLane;				// lane number	
	std::string strDeviceAddress;		// device address
};

const CString strEventList[] = {
	"Show All",
	"Junction",
	"RunRedLight",
	"OverLine", 
	"Retrograde", 
	"OverSpeed", 
	"UnderSpeed",
	"Parking",
	"WrongRoute",
	"CrossLane",
	"OverYellowLine",
	"YellowPlateInLane",
	"VehicleInRoute",
	"ManualSnap",
	"PedestrainPriority",
	"VehicleInBusRoute",
	"WithOutSafebelt",
	"TrafficJam", 
	"Pedestrain", 
	"TurnLeft", 
	"TurnRight", 
	"U-Turn",
	"OverStopLine", 
	"Smoking",
	"Calling", 
	"TurnLeftAfterStraight", 
	"QueueJump", 
	"TurnRightAfterStraight", 
	"TurnRightAfterPeople", 
	"TruckForbid", 
	"PedestrianJunction", 
	"NonMotorInmotorRoute",
	"NonMotorOverLoad", 
	"NonMotorWithOutSafeHat", 
	"NonMotorHoldUmbrella", 
	"BigBendSmallTurn"
};
const int nEventTypeList[]={
	EVENT_IVS_ALL, 
	EVENT_IVS_TRAFFICJUNCTION, 
	EVENT_IVS_TRAFFIC_RUNREDLIGHT, 
	EVENT_IVS_TRAFFIC_OVERLINE,
	EVENT_IVS_TRAFFIC_RETROGRADE,
	EVENT_IVS_TRAFFIC_OVERSPEED,
	EVENT_IVS_TRAFFIC_UNDERSPEED,
	EVENT_IVS_TRAFFIC_PARKING,
	EVENT_IVS_TRAFFIC_WRONGROUTE,
	EVENT_IVS_TRAFFIC_CROSSLANE,
	EVENT_IVS_TRAFFIC_OVERYELLOWLINE,	
	EVENT_IVS_TRAFFIC_YELLOWPLATEINLANE,
	EVENT_IVS_TRAFFIC_VEHICLEINROUTE,
	EVENT_IVS_TRAFFIC_MANUALSNAP,
	EVENT_IVS_TRAFFIC_PEDESTRAINPRIORITY,
	EVENT_IVS_TRAFFIC_VEHICLEINBUSROUTE,
	EVENT_IVS_TRAFFIC_WITHOUT_SAFEBELT,
	EVENT_IVS_TRAFFICJAM,
	EVENT_IVS_TRAFFIC_PEDESTRAIN,
	EVENT_IVS_TRAFFIC_TURNLEFT,
	EVENT_IVS_TRAFFIC_TURNRIGHT,
	EVENT_IVS_TRAFFIC_UTURN,
	EVENT_IVS_TRAFFIC_OVERSTOPLINE,
	EVENT_IVS_TRAFFIC_DRIVER_SMOKING,
	EVENT_IVS_TRAFFIC_DRIVER_CALLING,
	EVENT_IVS_TRAFFIC_TURNLEFTAFTERSTRAIGHT,
	EVENT_IVS_TRAFFIC_QUEUEJUMP,
	EVENT_IVS_TRAFFIC_TURNRIGHTAFTERSTRAIGHT,
	EVENT_IVS_TRAFFIC_TURNRIGHTAFTERPEOPLE,
	EVENT_IVS_TRAFFIC_TRUCKFORBID,
	EVENT_IVS_PEDESTRIAN_JUNCTION,
	EVENT_IVS_TRAFFIC_NONMOTORINMOTORROUTE,
	EVENT_IVS_TRAFFIC_NONMOTOR_OVERLOAD,
	EVENT_IVS_TRAFFIC_NONMOTOR_WITHOUTSAFEHAT,
	EVENT_IVS_TRAFFIC_NONMOTOR_HOLDUMBRELLA,
	EVENT_IVS_TRAFFIC_BIGBENDSMALLTURN
};

const CString strPlateTypeList[] = {
	"Unknown",
	"Normal",
	"YellowPlate",
	"DoubleYellow",
	"Police",
	"WJ",
	"OuterGuard",
	"DoubleOuterGuard",
	"SpecialAdministrativeRegion",
	"Trainning",
	"Personal",
	"Agri",
	"Embassy",
	"Moto",
	"Tractor",
	"OfficialCar",
	"PersonalCar",
	"WarCar",
	"Other",
	"CivilAviation",
	"Black",
	"PureNewEnergyMicroCar",
	"MixedNewEnergyMicroCar",
	"PureNewEnergyLargeCar",
	"MixedNewEnergyLargeCar"
};
const int nPlateTypeList[]={
	NET_PLATE_TYPE_UNKNOWN,                             // Unknown
	NET_PLATE_TYPE_NORMAL,                              // "Normal" 
	NET_PLATE_TYPE_YELLOW,                              // "Yellow" 
	NET_PLATE_TYPE_DOUBLEYELLOW,                        // "DoubleYellow" 
	NET_PLATE_TYPE_POLICE,                              // "Police" 
	NET_PLATE_TYPE_WJ,									// "WJ" 
	NET_PLATE_TYPE_OUTERGUARD,                          // "OuterGuard" 
	NET_PLATE_TYPE_DOUBLEOUTERGUARD,                    // "DoubleOuterGuard" 
	NET_PLATE_TYPE_SAR,                                 // "SAR" 
	NET_PLATE_TYPE_TRAINNING,                           // "Trainning" 
	NET_PLATE_TYPE_PERSONAL,                            // "Personal" 
	NET_PLATE_TYPE_AGRI,                                // "Agri" 
	NET_PLATE_TYPE_EMBASSY,                             // "Embassy" 
	NET_PLATE_TYPE_MOTO,                                // "Moto" 
	NET_PLATE_TYPE_TRACTOR,                             // "Tractor" 
	NET_PLATE_TYPE_OFFICIALCAR,                         // "OfficialCar " 
	NET_PLATE_TYPE_PERSONALCAR,                         // "PersonalCar" 
	NET_PLATE_TYPE_WARCAR,                              // "WarCar"  
	NET_PLATE_TYPE_OTHER,                               // "Other" 
	NET_PLATE_TYPE_CIVILAVIATION,                       // "Civilaviation" 
	NET_PLATE_TYPE_BLACK,                               // "Black" 
	NET_PLATE_TYPE_PURENEWENERGYMICROCAR,               // "PureNewEnergyMicroCar" 
	NET_PLATE_TYPE_MIXEDNEWENERGYMICROCAR,              // "MixedNewEnergyMicroCar" 
	NET_PLATE_TYPE_PURENEWENERGYLARGECAR,               // "PureNewEnergyLargeCar" 
	NET_PLATE_TYPE_MIXEDNEWENERGYLARGECAR,              // "MixedNewEnergyLargeCar" 
};

const CString strPlateColorTypeList[] = {
	"Other",
	"BlueColor",
	"YellowColor",
	"White",
	"Black",
	"YellowBottomBlackText",
	"BlueBottomWhiteText",
	"BlackBottomWhiteText",
	"ShadowGreen",
	"YellowGreen"
};
const int nPlateColorTypeList[]={
	NET_PLATE_COLOR_OTHER,                              // Other
	NET_PLATE_COLOR_BLUE,                               // "Blue"
	NET_PLATE_COLOR_YELLOW,                             // "Yellow"    
	NET_PLATE_COLOR_WHITE,                              // "White"
	NET_PLATE_COLOR_BLACK,                              // "Black"
	NET_PLATE_COLOR_YELLOW_BOTTOM_BLACK_TEXT,           // "YellowbottomBlackText"
	NET_PLATE_COLOR_BLUE_BOTTOM_WHITE_TEXT,             // "BluebottomWhiteText" 
	NET_PLATE_COLOR_BLACK_BOTTOM_WHITE_TEXT,            // "BlackBottomWhiteText"
	NET_PLATE_COLOR_SHADOW_GREEN,						// "ShadowGreen"
	NET_PLATE_COLOR_YELLOW_GREEN,						// "YellowGreen"
};

const CString strVehtcleTypeList[] = {
	"Unknown",
	"Motor",
	"Non-Motor",
	"Bus",
	"Bicycle",
	"Motorcycle",
	"UnlicensedMotor",
	"LargeCar",
	"MicroCar",
	"EmbassyCar",
	"MarginalCar",
	"AreaoutCar",
	"ForeignCar",
	"DualTriWheelMotorcycle",
	"LightMotorcycle",
	"EmbassyMotorcycle",
	"MarginalMotorcycle",
	"AreaoutMotorcycle",
	"ForeignMotorcycle",
	"FarmTransmitCar",
	"Tractor",
	"Trailer",
	"CoachCar",
	"CoachMotorcycle",
	"TrialCar",
	"TrialMotorcycle",
	"TemporaryEntryCar",
	"TemporaryEntryMotorcycle",
	"TemporarySteerCar",
	"PassengerCar",
	"LargeTruck",
	"MidTruck",
	"SaloonCar",
	"Microbus",
	"MicroTruck",
	"Tricycle",
	"Passerby"
};
const int nVehtcleTypeList[]={
	NET_VEHICLE_TYPE_UNKNOW,                            // Unknown
	NET_VEHICLE_TYPE_MOTOR,                             // "Motor" 
	NET_VEHICLE_TYPE_NON_MOTOR,                         // "Non-Motor"
	NET_VEHICLE_TYPE_BUS,                               // "Bus"
	NET_VEHICLE_TYPE_BICYCLE,                           // "Bicycle" 
	NET_VEHICLE_TYPE_MOTORCYCLE,                        // "Motorcycle"
	NET_VEHICLE_TYPE_UNLICENSEDMOTOR,                   // "UnlicensedMotor": 
	NET_VEHICLE_TYPE_LARGECAR,                          // "LargeCar"  
	NET_VEHICLE_TYPE_MICROCAR,                          // "MicroCar" 
	NET_VEHICLE_TYPE_EMBASSYCAR,                        // "EmbassyCar" 
	NET_VEHICLE_TYPE_MARGINALCAR,                       // "MarginalCar" 
	NET_VEHICLE_TYPE_AREAOUTCAR,                        // "AreaoutCar" 
	NET_VEHICLE_TYPE_FOREIGNCAR,                        // "ForeignCar" 
	NET_VEHICLE_TYPE_DUALTRIWHEELMOTORCYCLE,            // "DualTriWheelMotorcycle"
	NET_VEHICLE_TYPE_LIGHTMOTORCYCLE,                   // "LightMotorcycle" 
	NET_VEHICLE_TYPE_EMBASSYMOTORCYCLE,                 // "EmbassyMotorcycle "
	NET_VEHICLE_TYPE_MARGINALMOTORCYCLE,                // "MarginalMotorcycle "
	NET_VEHICLE_TYPE_AREAOUTMOTORCYCLE,                 // "AreaoutMotorcycle "
	NET_VEHICLE_TYPE_FOREIGNMOTORCYCLE,                 // "ForeignMotorcycle "
	NET_VEHICLE_TYPE_FARMTRANSMITCAR,                   // "FarmTransmitCar" 
	NET_VEHICLE_TYPE_TRACTOR,                           // "Tractor" 
	NET_VEHICLE_TYPE_TRAILER,                           // "Trailer"  
	NET_VEHICLE_TYPE_COACHCAR,                          // "CoachCar"
	NET_VEHICLE_TYPE_COACHMOTORCYCLE,                   // "CoachMotorcycle "
	NET_VEHICLE_TYPE_TRIALCAR,                          // "TrialCar" 
	NET_VEHICLE_TYPE_TRIALMOTORCYCLE,                   // "TrialMotorcycle "
	NET_VEHICLE_TYPE_TEMPORARYENTRYCAR,                 // "TemporaryEntryCar"
	NET_VEHICLE_TYPE_TEMPORARYENTRYMOTORCYCLE,          // "TemporaryEntryMotorcycle"
	NET_VEHICLE_TYPE_TEMPORARYSTEERCAR,                 // "TemporarySteerCar"
	NET_VEHICLE_TYPE_PASSENGERCAR,                      // "PassengerCar" 
	NET_VEHICLE_TYPE_LARGETRUCK,                        // "LargeTruck" 
	NET_VEHICLE_TYPE_MIDTRUCK,                          // "MidTruck" 
	NET_VEHICLE_TYPE_SALOONCAR,                         // "SaloonCar" 
	NET_VEHICLE_TYPE_MICROBUS,                          // "Microbus"
	NET_VEHICLE_TYPE_MICROTRUCK,                        // "MicroTruck"
	NET_VEHICLE_TYPE_TRICYCLE,                          // "Tricycle"
	NET_VEHICLE_TYPE_PASSERBY,                          // "Passerby" 
};

const CString strVehtcleColorTypeList[] = {
	"Other",
	"White",
	"Black",
	"RedColor",
	"YellowColor",
	"Gray",
	"BlueColor",
	"Green",
	"Pink",
	"Purple",
	"Brown"
};
const int nVehtcleColorTypeList[]={
	NET_VEHICLE_COLOR_OTHER,                            // Other
	NET_VEHICLE_COLOR_WHITE,                            // "White"
	NET_VEHICLE_COLOR_BLACK,                            // "Black"
	NET_VEHICLE_COLOR_RED,                              // "Red"
	NET_VEHICLE_COLOR_YELLOW,                           // "Yellow"
	NET_VEHICLE_COLOR_GRAY,                             // "Gray"
	NET_VEHICLE_COLOR_BLUE,                             // "Blue"
	NET_VEHICLE_COLOR_GREEN,                            // "Green"
	NET_VEHICLE_COLOR_PINK,                             // "Pink"
	NET_VEHICLE_COLOR_PURPLE,                           // "Purple"
	NET_VEHICLE_COLOR_BROWN,                            // "Brown"
};

const CString strCarControlTypeList[] = {
	"Other",
	"OverdueNoCheck",
	"BrigandageCar",
	"Breaking",
	"CausetroubleEscape"
};
const int nCarControlTypeList[]={
	NET_CAR_CONTROL_OTHER,
	NET_CAR_CONTROL_OVERDUE_NO_CHECK,                   // "OverdueNoCheck"
	NET_CAR_CONTROL_BRIGANDAGE_CAR,                     // "BrigandageCar"
	NET_CAR_CONTROL_BREAKING,                           // "Breaking"
	NET_CAR_CONTROL_CAUSETROUBLE_ESCAPE,                // "CausetroubleEscape"
};

void  g_SetWndStaticText(CWnd * pWnd);

std::string GetDataFolder();
BOOL IsTypeHasLP(DWORD dwEventType);

class CRealLoadPictureApp : public CWinApp
{
public:
	CRealLoadPictureApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRealLoadPictureApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL
// Implementation

	//{{AFX_MSG(CRealLoadPictureApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
inline ColDes ColDesObj(char* szColTitle, const int nColWidth)
{
	ColDes colDes;
	colDes.strColTitle = ConvertStr(szColTitle);
	colDes.nColWidth = nColWidth;
	return colDes;
}

struct StuEventInfo
{
	int					nStructSize;			// event struct size
	EVENT_INFO_DISPLAY  stuEventInfoToDisplay;	// Event info to display
	std::string			strDeviceAddress;		// event address
	DH_RECT				rectBoundingBox;		
	DWORD				dwOffset;
	DWORD				dwOffsetLength;
	std::string			strUTCTime;

	StuEventInfo() : nStructSize(0), strDeviceAddress(""), dwOffset(0), dwOffsetLength(0), strUTCTime("")
	{		
		rectBoundingBox.bottom = 0;
		rectBoundingBox.left = 0;
		rectBoundingBox.right = 0;
		rectBoundingBox.top = 0;
	}
};

BOOL GetStructInfo(DWORD nEventType, void* pEventInfo,int nGetWhat, StuEventInfo& stuEventInfo);
std::string EventType2Str(int nEventType);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_REALLOADPICTURE_H__D79C176B_B614_478F_A246_6DACDBE3869F__INCLUDED_)
