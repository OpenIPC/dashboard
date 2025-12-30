// IntelligentTrafficCamera.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "IntelligentTrafficCamera.h"
#include "IntelligentTrafficCameraDlg.h"
#include "Utility.h"
#include <atlconv.h>
#include <assert.h>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE

static char THIS_FILE[] = __FILE__;
#endif

#ifdef WIN32
#pragma warning (disable: 4514 4786)
#endif

using std::string;

//Set static text for dialog box(English->current language)
void g_SetWndStaticText(CWnd * pWnd)
{
	CString strCaption,strText;
	
	//Set main window title 
	pWnd->GetWindowText(strCaption);
	if(strCaption.GetLength()>0)
	{
		strText=ConvertString(strCaption);
		pWnd->SetWindowText(strText);
	}
	
	//Set small window title
	CWnd * pChild=pWnd->GetWindow(GW_CHILD);
	CString strClassName;
	while(pChild)
	{
		//////////////////////////////////////////////////////////////////////////		
		//Added by Jackbin 2005-03-11
		strClassName = ((CRuntimeClass*)pChild->GetRuntimeClass())->m_lpszClassName;
		if(strClassName == "CEdit")
		{
			//The next small window 
			pChild=pChild->GetWindow(GW_HWNDNEXT);
			continue;
		}	
		
		//Set current language text for small window. 
		pChild->GetWindowText(strCaption);
		strText=ConvertString(strCaption);
		pChild->SetWindowText(strText);
		
		//Next small window 
		pChild=pChild->GetWindow(GW_HWNDNEXT);
	}
}

BOOL IsTypeHasLP(DWORD dwEventType)
{
	for (int i = 1; i < sizeof(nEventTypeList)/sizeof(int); i++)
	{	
		if (dwEventType == nEventTypeList[i])
		{
			return TRUE;
		}
	}
	return FALSE;
}


std::string GetDataFolder()
{
	std::string strCfgFilePath;
	std::string strCfgDefaultFolder;
	GetModulePath("dhnetsdk.dll", strCfgFilePath);
	strCfgDefaultFolder = strCfgFilePath;
	strCfgFilePath.append(CFG_CFG_FILE_NAME);
	strCfgDefaultFolder.append(PC_DATA_STORE_FOLDER);
	char szFolder[1024] = {0};
	::GetPrivateProfileString(CFG_SECTION_CFG, CFG_KEY_DATA_PATH, strCfgDefaultFolder.c_str(), szFolder, 1024, strCfgFilePath.c_str());
	return std::string(szFolder);
}


/////////////////////////////////////////////////////////////////////////////
// CRealLoadPictureApp

BEGIN_MESSAGE_MAP(CRealLoadPictureApp, CWinApp)
//{{AFX_MSG_MAP(CRealLoadPictureApp)
// NOTE - the ClassWizard will add and remove mapping macros here.
//    DO NOT EDIT what you see in these blocks of generated code!
//}}AFX_MSG
ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRealLoadPictureApp construction

CRealLoadPictureApp::CRealLoadPictureApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CRealLoadPictureApp object

CRealLoadPictureApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CRealLoadPictureApp initialization

BOOL CRealLoadPictureApp::InitInstance()
{
	AfxEnableControlContainer();
	
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
	
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif	
	
	CRealLoadPictureDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

BOOL GetStructInfo(DWORD nEventType, void* pEventInfo, int nGetWhat, StuEventInfo& stuEventInfo)				   
{
	if (NULL == pEventInfo)
	{
		return FALSE;
	}

	switch(nEventType)
	{		
	case EVENT_IVS_TRAFFICJUNCTION:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFICJUNCTION_INFO);
			}			

			DEV_EVENT_TRAFFICJUNCTION_INFO* pInfo = (DEV_EVENT_TRAFFICJUNCTION_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{					
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";// set "0" as default
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				
				
				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;
				
				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}	

			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}

			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}

			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;			
	case EVENT_IVS_TRAFFIC_RUNREDLIGHT:
		{
			//Size
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_RUNREDLIGHT_INFO);
			}

			DEV_EVENT_TRAFFIC_RUNREDLIGHT_INFO* pInfo = (DEV_EVENT_TRAFFIC_RUNREDLIGHT_INFO*)pEventInfo;

			//Content
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}	
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}

			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_OVERLINE:
		{
			//Size
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_OVERLINE_INFO);
			}

			DEV_EVENT_TRAFFIC_OVERLINE_INFO* pInfo = (DEV_EVENT_TRAFFIC_OVERLINE_INFO*)pEventInfo;			
			//Content
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);			

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}	
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_RETROGRADE:
		{
			//Size
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_RETROGRADE_INFO);
			}

			DEV_EVENT_TRAFFIC_RETROGRADE_INFO* pInfo = (DEV_EVENT_TRAFFIC_RETROGRADE_INFO*)pEventInfo;			
			//Content
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);	

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}	
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_OVERSPEED:
		{
			//Size
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_OVERSPEED_INFO);
			}

			DEV_EVENT_TRAFFIC_OVERSPEED_INFO* pInfo = (DEV_EVENT_TRAFFIC_OVERSPEED_INFO*)pEventInfo;			
			//Content
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_UNDERSPEED:
		{
			//Size
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_UNDERSPEED_INFO);
			}

			DEV_EVENT_TRAFFIC_UNDERSPEED_INFO* pInfo = (DEV_EVENT_TRAFFIC_UNDERSPEED_INFO*)pEventInfo;			
			//Content
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);	

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;	
	case EVENT_IVS_TRAFFIC_PARKING:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_PARKING_INFO);
			}

			DEV_EVENT_TRAFFIC_PARKING_INFO* pInfo = (DEV_EVENT_TRAFFIC_PARKING_INFO*)pEventInfo;			
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_WRONGROUTE:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_WRONGROUTE_INFO);
			}

			DEV_EVENT_TRAFFIC_WRONGROUTE_INFO* pInfo = (DEV_EVENT_TRAFFIC_WRONGROUTE_INFO*)pEventInfo;			
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);	

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_CROSSLANE:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_CROSSLANE_INFO);
			}

			DEV_EVENT_TRAFFIC_CROSSLANE_INFO* pInfo = (DEV_EVENT_TRAFFIC_CROSSLANE_INFO*)pEventInfo;			
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stuTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stuTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stuTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stuTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stuTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stuTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stuTrafficCar.szDeviceAddress) ?  "" : pInfo->stuTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_OVERYELLOWLINE:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_OVERYELLOWLINE_INFO);
			}

			DEV_EVENT_TRAFFIC_OVERYELLOWLINE_INFO* pInfo = (DEV_EVENT_TRAFFIC_OVERYELLOWLINE_INFO*)pEventInfo;			
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);	

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;	
	case EVENT_IVS_TRAFFIC_YELLOWPLATEINLANE:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_YELLOWPLATEINLANE_INFO);
			}

			DEV_EVENT_TRAFFIC_YELLOWPLATEINLANE_INFO* pInfo = (DEV_EVENT_TRAFFIC_YELLOWPLATEINLANE_INFO*)pEventInfo;			
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;	
	
	case EVENT_IVS_TRAFFIC_VEHICLEINROUTE:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_VEHICLEINROUTE_INFO);
			}

			DEV_EVENT_TRAFFIC_VEHICLEINROUTE_INFO* pInfo = (DEV_EVENT_TRAFFIC_VEHICLEINROUTE_INFO*)pEventInfo;			
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_MANUALSNAP:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_MANUALSNAP_INFO);
			}

			DEV_EVENT_TRAFFIC_MANUALSNAP_INFO* pInfo = (DEV_EVENT_TRAFFIC_MANUALSNAP_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;

	case EVENT_IVS_TRAFFIC_PEDESTRAINPRIORITY:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_PEDESTRAINPRIORITY_INFO);
			}

			DEV_EVENT_TRAFFIC_PEDESTRAINPRIORITY_INFO* pInfo = (DEV_EVENT_TRAFFIC_PEDESTRAINPRIORITY_INFO*)pEventInfo;

			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);	

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_VEHICLEINBUSROUTE:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_VEHICLEINBUSROUTE_INFO);
			}

			DEV_EVENT_TRAFFIC_VEHICLEINBUSROUTE_INFO* pInfo = (DEV_EVENT_TRAFFIC_VEHICLEINBUSROUTE_INFO*)pEventInfo;

			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);	

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_WITHOUT_SAFEBELT:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_WITHOUT_SAFEBELT);
			}

			DEV_EVENT_TRAFFIC_WITHOUT_SAFEBELT* pInfo = (DEV_EVENT_TRAFFIC_WITHOUT_SAFEBELT*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stuTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stuTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stuTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stuTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stuTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stuTrafficCar.nLane);	

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stuTrafficCar.szDeviceAddress) ?  "" : pInfo->stuTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFICJAM:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFICJAM_INFO);
			}

			DEV_EVENT_TRAFFICJAM_INFO* pInfo = (DEV_EVENT_TRAFFICJAM_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				//stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stCommInfo.szObjectSubType;
				//stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stTrafficCar.sz
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);	

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				//stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stuTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				//stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				//stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				//stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_PEDESTRAIN: 
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_PEDESTRAIN_INFO);
			}

			DEV_EVENT_TRAFFIC_PEDESTRAIN_INFO* pInfo = (DEV_EVENT_TRAFFIC_PEDESTRAIN_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				


				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_TURNLEFT:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_TURNLEFT_INFO);
			}

			DEV_EVENT_TRAFFIC_TURNLEFT_INFO* pInfo = (DEV_EVENT_TRAFFIC_TURNLEFT_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_TURNRIGHT:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_TURNRIGHT_INFO);
			}

			DEV_EVENT_TRAFFIC_TURNRIGHT_INFO* pInfo = (DEV_EVENT_TRAFFIC_TURNRIGHT_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_UTURN:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_UTURN_INFO);
			}

			DEV_EVENT_TRAFFIC_UTURN_INFO* pInfo = (DEV_EVENT_TRAFFIC_UTURN_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_OVERSTOPLINE:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_OVERSTOPLINE);
			}

			DEV_EVENT_TRAFFIC_OVERSTOPLINE* pInfo = (DEV_EVENT_TRAFFIC_OVERSTOPLINE*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stuTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stuTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stuTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stuTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stuTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stuTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stuTrafficCar.szDeviceAddress) ?  "" : pInfo->stuTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_DRIVER_SMOKING:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_DRIVER_SMOKING);
			}

			DEV_EVENT_TRAFFIC_DRIVER_SMOKING* pInfo = (DEV_EVENT_TRAFFIC_DRIVER_SMOKING*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stuTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stuTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stuTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stuTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stuTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stuTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stuTrafficCar.szDeviceAddress) ?  "" : pInfo->stuTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_DRIVER_CALLING:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_DRIVER_CALLING);
			}

			DEV_EVENT_TRAFFIC_DRIVER_CALLING* pInfo = (DEV_EVENT_TRAFFIC_DRIVER_CALLING*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stuTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stuTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stuTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stuTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stuTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stuTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stuTrafficCar.szDeviceAddress) ?  "" : pInfo->stuTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_TURNLEFTAFTERSTRAIGHT:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TURNLEFTAFTERSTRAIGHT_INFO);
			}

			DEV_EVENT_TURNLEFTAFTERSTRAIGHT_INFO* pInfo = (DEV_EVENT_TURNLEFTAFTERSTRAIGHT_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_QUEUEJUMP:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_QUEUEJUMP_INFO);
			}

			DEV_EVENT_TRAFFIC_QUEUEJUMP_INFO* pInfo = (DEV_EVENT_TRAFFIC_QUEUEJUMP_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_TURNRIGHTAFTERSTRAIGHT:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TURNRIGHTAFTERSTRAIGHT_INFO);
			}

			DEV_EVENT_TURNRIGHTAFTERSTRAIGHT_INFO* pInfo = (DEV_EVENT_TURNRIGHTAFTERSTRAIGHT_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->stuUTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->stuUTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_TURNRIGHTAFTERPEOPLE:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TURNRIGHTAFTERPEOPLE_INFO);
			}

			DEV_EVENT_TURNRIGHTAFTERPEOPLE_INFO* pInfo = (DEV_EVENT_TURNRIGHTAFTERPEOPLE_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->stuUTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->stuUTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_TRUCKFORBID:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFICTRUCKFORBID_INFO);
			}

			DEV_EVENT_TRAFFICTRUCKFORBID_INFO* pInfo = (DEV_EVENT_TRAFFICTRUCKFORBID_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_PEDESTRIAN_JUNCTION:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_PEDESTRIAN_JUNCTION_INFO);
			}

			DEV_EVENT_PEDESTRIAN_JUNCTION_INFO* pInfo = (DEV_EVENT_PEDESTRIAN_JUNCTION_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_NONMOTORINMOTORROUTE:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_NONMOTORINMOTORROUTE_INFO);
			}

			DEV_EVENT_TRAFFIC_NONMOTORINMOTORROUTE_INFO* pInfo = (DEV_EVENT_TRAFFIC_NONMOTORINMOTORROUTE_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_NONMOTOR_OVERLOAD:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_NONMOTOR_OVERLOAD_INFO);
			}

			DEV_EVENT_TRAFFIC_NONMOTOR_OVERLOAD_INFO* pInfo = (DEV_EVENT_TRAFFIC_NONMOTOR_OVERLOAD_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_NONMOTOR_WITHOUTSAFEHAT:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_NONMOTOR_WITHOUTSAFEHAT_INFO);
			}

			DEV_EVENT_TRAFFIC_NONMOTOR_WITHOUTSAFEHAT_INFO* pInfo = (DEV_EVENT_TRAFFIC_NONMOTOR_WITHOUTSAFEHAT_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_NONMOTOR_HOLDUMBRELLA:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_TRAFFIC_NONMOTOR_HOLDUMBRELLA_INFO);
			}

			DEV_EVENT_TRAFFIC_NONMOTOR_HOLDUMBRELLA_INFO* pInfo = (DEV_EVENT_TRAFFIC_NONMOTOR_HOLDUMBRELLA_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	case EVENT_IVS_TRAFFIC_BIGBENDSMALLTURN:
		{
			if (nGetWhat & GET_STRUCT_SIZE)
			{
				stuEventInfo.nStructSize = sizeof(DEV_EVENT_BIGBENDSMALLTURN_INFO);
			}

			DEV_EVENT_BIGBENDSMALLTURN_INFO* pInfo = (DEV_EVENT_BIGBENDSMALLTURN_INFO*)pEventInfo;
			if (nGetWhat & GET_EVENT_CONTENT)
			{
				stuEventInfo.stuEventInfoToDisplay.strCountNum = "0";
				stuEventInfo.stuEventInfoToDisplay.strTime = NetTimeEx2Str(pInfo->UTC);
				stuEventInfo.stuEventInfoToDisplay.strEventType = EventType2Str(nEventType);
				stuEventInfo.stuEventInfoToDisplay.strFileIndex =Int2Str(pInfo->stuFileInfo.bIndex);
				stuEventInfo.stuEventInfoToDisplay.strFileCount =Int2Str(pInfo->stuFileInfo.bCount);
				stuEventInfo.stuEventInfoToDisplay.strGroupId = Int2Str(pInfo->stuFileInfo.nGroupId);				

				stuEventInfo.stuEventInfoToDisplay.strPlateNumber = pInfo->stTrafficCar.szPlateNumber ;
				stuEventInfo.stuEventInfoToDisplay.strPlateColor = pInfo->stTrafficCar.szPlateColor;
				stuEventInfo.stuEventInfoToDisplay.strPlateType = pInfo->stTrafficCar.szPlateType;
				stuEventInfo.stuEventInfoToDisplay.strVehicleType = pInfo->stuVehicle.szObjectSubType;			
				stuEventInfo.stuEventInfoToDisplay.strVehicleColor = pInfo->stTrafficCar.szVehicleColor;

				stuEventInfo.stuEventInfoToDisplay.strVehicleSize = VehicleSizeToString(pInfo->stTrafficCar.nVehicleSize);
				stuEventInfo.stuEventInfoToDisplay.strLane  = Int2Str(pInfo->stTrafficCar.nLane);

				stuEventInfo.strUTCTime = NetTimeEx2StrEx(pInfo->UTC);
			}
			if (nGetWhat & GET_DEVICE_ADDRESS)
			{
				stuEventInfo.strDeviceAddress = (NULL == pInfo->stTrafficCar.szDeviceAddress) ?  "" : pInfo->stTrafficCar.szDeviceAddress;
			}
			if (nGetWhat & GET_PICTURE_RECT)
			{
				stuEventInfo.rectBoundingBox = pInfo->stuObject.BoundingBox;
			}
			if (nGetWhat & GET_PLATE_PICTURE_INFO)
			{
				stuEventInfo.dwOffset		= pInfo->stuObject.stPicInfo.dwOffSet;
				stuEventInfo.dwOffsetLength = pInfo->stuObject.stPicInfo.dwFileLenth;
			}
		}
		break;
	default:
		return FALSE;	
	}		
	return TRUE;				
}

std::string EventType2Str(int nEventType)
{
	std::string strType = ConvertString("Unknown");	
	for (int i = 1; i < sizeof(nEventTypeList)/sizeof(int); i++)
	{	
		if (nEventType == nEventTypeList[i])
		{
			strType = ConvertString(strEventList[i]);
		}
	}
	return strType;
}
