// BlackWhiltListDlg.cpp :
//

#include "stdafx.h"
#include "EntranceAccessDevices.h"
#include "BlackWhiltListDlg.h"

void CALLBACK  bfTransFileCallBack(LLONG lHandle, int nTransType, int nState, int nSendSize, int nTotalSize, LDWORD dwUser)
{
	BlackWhiltListDlg* dlg = (BlackWhiltListDlg*)dwUser;
	if (nTransType == DH_DEV_BLACKWHITE_LOAD)
	{
		if (nState == 0 && nSendSize == -1)
		{
			dlg->PostMessage(WM_STOP_EXPORT, 0, 0);
		}
	} 
	else if(nTransType == DH_DEV_BLACKWHITETRANS_SEND)
	{
		if (nState == 0 && nSendSize == -1)
		{
			dlg->PostMessage(WM_STOP_IMPORT, 0, 0);
		}
	}
	dlg->PostMessage(WM_TRANSMIT_EVENT, (WPARAM)nSendSize, (LPARAM)nTotalSize);
}

IMPLEMENT_DYNAMIC(BlackWhiltListDlg, CDialog)

BlackWhiltListDlg::BlackWhiltListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BlackWhiltListDlg::IDD, pParent)
{
	m_emchangeType = EM_UNKNOWN;
}

BlackWhiltListDlg::~BlackWhiltListDlg()
{

}

void BlackWhiltListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_LIST, m_combo_list);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
}

BEGIN_MESSAGE_MAP(BlackWhiltListDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT, &BlackWhiltListDlg::OnBnClickedButtonImport)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, &BlackWhiltListDlg::OnBnClickedButtonExport)
	ON_MESSAGE(WM_TRANSMIT_EVENT, OnTransmitEvent)
	ON_MESSAGE(WM_STOP_EXPORT, OnStopExport)
	ON_MESSAGE(WM_STOP_IMPORT, OnStopImport)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL BlackWhiltListDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	m_combo_list.AddString(ConvertString("Blocklist"));
	m_combo_list.AddString(ConvertString("WhiltList"));
	m_combo_list.SetCurSel(0);

	GetDlgItem(IDC_EDIT_PATH_IMPORT)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_PATH_EXPORT)->EnableWindow(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void BlackWhiltListDlg::OnBnClickedButtonImport()
{
	if (m_emchangeType == EM_UNKNOWN)
	{
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS1))->SetPos(0);
		CString strFilter = "List File(*.CSV)|*.CSV|All Files|*.*|";
		CFileDialog dlg(TRUE, NULL,NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, strFilter, this);

		if(dlg.DoModal() == IDOK)
		{
			CString strPath = dlg.GetPathName();
			SetDlgItemText(IDC_EDIT_PATH_IMPORT,strPath);

			DHDEV_BLACKWHITE_LIST_INFO stulistinfo; 
			strncpy(stulistinfo.szFile, strPath.GetBuffer(), sizeof(stulistinfo.szFile)-1);
			stulistinfo.byFileType = ((CComboBox*)GetDlgItem(IDC_COMBO_LIST))->GetCurSel();
			stulistinfo.byAction = 0;
			BOOL bret = Device::GetInstance().StartImportList((char*)&stulistinfo,sizeof(DHDEV_BLACKWHITE_LIST_INFO),bfTransFileCallBack,(LDWORD)this);
			if (!bret)
			{
				MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
			}
			else
			{
				ChangeUI(EM_IMPORT_LIST_START);
			}
		}
	} 
	else if(m_emchangeType == EM_IMPORT_LIST_START)
	{
		StopSendList();
	}
}

void BlackWhiltListDlg::OnBnClickedButtonExport()
{
	if (m_emchangeType == EM_UNKNOWN)
	{
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS1))->SetPos(0);
		CFileDialog dlg(FALSE,"*.CSV","list.CSV",OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY, "List File(*.CSV)|*.*||",this);
		if(dlg.DoModal() == IDOK)
		{	
			CString strPath = dlg.GetPathName();
			SetDlgItemText(IDC_EDIT_PATH_EXPORT,strPath);

			DHDEV_LOAD_BLACKWHITE_LIST_INFO stulistinfo;
			strncpy(stulistinfo.szFile, strPath.GetBuffer(), sizeof(stulistinfo.szFile)-1);
			stulistinfo.byFileType = ((CComboBox*)GetDlgItem(IDC_COMBO_LIST))->GetCurSel();

			BOOL bret = Device::GetInstance().StartExportList((char*)&stulistinfo,sizeof(DHDEV_LOAD_BLACKWHITE_LIST_INFO),bfTransFileCallBack,(LDWORD)this);
			if (!bret)
			{
				MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
			}
			else
			{
				ChangeUI(EM_EXPORT_LIST_START);
			}
		}
	} 
	else if(m_emchangeType == EM_EXPORT_LIST_START)
	{
		StopLoadList();
	}
}

LRESULT BlackWhiltListDlg::OnTransmitEvent(WPARAM wParam, LPARAM lParam)
{
	int nSendSize = (int)wParam;
	int nTotalSize = (int)lParam;
	if (nSendSize >= 0 && nTotalSize > 0 && nTotalSize >= nSendSize)
	{
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS1))->SetRange32(0,nTotalSize);
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS1))->SetPos(nSendSize);
	}
	if (nSendSize < 0)
	{
		MessageBox(ConvertString("Transmit Complete!"),ConvertString("Prompt"));
		ChangeUI(EM_UNKNOWN);
	}
	return 0;
}

LRESULT BlackWhiltListDlg::OnStopExport(WPARAM wParam, LPARAM lParam)
{
	Device::GetInstance().StopExportList();
	return 0;
}

LRESULT BlackWhiltListDlg::OnStopImport(WPARAM wParam, LPARAM lParam)
{
	Device::GetInstance().StopImportList();
	return 0;
}

void BlackWhiltListDlg::ChangeUI(EM_TRAFFIC_LIST_OPERATOR_TYPE emchangeType)
{
	m_emchangeType = emchangeType;
	switch(m_emchangeType)
	{
	case EM_UNKNOWN:
		{
			GetDlgItem(IDC_BUTTON_IMPORT)->SetWindowText(ConvertString("Import"));
			GetDlgItem(IDC_BUTTON_EXPORT)->SetWindowText(ConvertString("Export"));
			SetWindowText(ConvertString("BlackWhiltList"));
			GetDlgItem(IDC_BUTTON_EXPORT)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_IMPORT)->EnableWindow(TRUE);
		}
		break;
	case EM_IMPORT_LIST_START:
		{
			GetDlgItem(IDC_BUTTON_IMPORT)->SetWindowText(ConvertString("Stop"));
			SetWindowText(ConvertString("Import"));
			GetDlgItem(IDC_BUTTON_EXPORT)->EnableWindow(FALSE);
		}
		break;
	case EM_EXPORT_LIST_START:
		{
			GetDlgItem(IDC_BUTTON_EXPORT)->SetWindowText(ConvertString("Stop"));
			SetWindowText(ConvertString("Export"));
			GetDlgItem(IDC_BUTTON_IMPORT)->EnableWindow(FALSE);
		}
		break;
	default:
		break;
	}
}

void BlackWhiltListDlg::StopLoadList()
{
	Device::GetInstance().StopExportList();
	ChangeUI(EM_UNKNOWN);
}

void BlackWhiltListDlg::StopSendList()
{
	Device::GetInstance().StopImportList();
	ChangeUI(EM_UNKNOWN);
}

void BlackWhiltListDlg::OnDestroy()
{
	CDialog::OnDestroy();
	if (m_emchangeType == EM_IMPORT_LIST_START)
	{
		StopSendList();
	} 
	else if(m_emchangeType == EM_EXPORT_LIST_START)
	{
		StopLoadList();
	}
}
