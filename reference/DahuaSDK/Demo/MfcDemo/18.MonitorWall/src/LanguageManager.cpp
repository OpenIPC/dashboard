#include "StdAfx.h"
#include "LanguageManager.h"
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#define LANGUAGE_FILE_ZHCN	_T("langchn.ini")


CLanguageManager::CLanguageManager(void)
{	
	SetLanguage(LANGUAGE_ZHCN);
}

CLanguageManager::~CLanguageManager(void)
{
}

CLanguageManager& CLanguageManager::Instance()
{
	static CLanguageManager langMgr;
	return langMgr;
}

void CLanguageManager::SetLanguage( int nLanguage )
{
	m_nLanguage = nLanguage;
	switch (nLanguage)
	{
	case LANGUAGE_EN:
		break;
	case LANGUAGE_ZHCN:
		{
			GetModuleFileName(NULL, m_szFilePath, _MAX_PATH);
			PathRemoveFileSpec(m_szFilePath);
			PathCombine(m_szFilePath, m_szFilePath, LANGUAGE_FILE_ZHCN);
		}
		break;
	}
}

CString CLanguageManager::ConvertString( LPCTSTR lpszText )
{
	CString strReturn;
	
	if (_tcslen(lpszText) > 0)
	{
		const TCHAR* kszSection = _T("String");
		const int knBufferSize = 256;
		GetPrivateProfileString(kszSection, lpszText, lpszText, strReturn.GetBuffer(knBufferSize), knBufferSize, m_szFilePath);
		strReturn.ReleaseBuffer();
		
		// \n转换成换行符
		strReturn.Replace("\\n", "\n");
	}
	
	return strReturn;
}

void CLanguageManager::UpdateWindowString( CWnd* pWnd )
{
	// 标题
	CString strCaption, strText;
	pWnd->GetWindowText(strCaption);
	if(!strCaption.IsEmpty())
	{
		strText = ConvertString(strCaption);
		pWnd->SetWindowText(strText);
	}

	// 控件
	CWnd* pChild = pWnd->GetWindow(GW_CHILD);
	while(pChild)
	{
		//////////////////////////////////////////////////////////////////////////				
		if(pChild->IsKindOf(RUNTIME_CLASS(CEdit)))
		{
			pChild = pChild->GetWindow(GW_HWNDNEXT);
			continue;
		}
		//////////////////////////////////////////////////////////////////////////	

		pChild->GetWindowText(strCaption);
		strText = ConvertString(strCaption);
		pChild->SetWindowText(strText);

		pChild = pChild->GetWindow(GW_HWNDNEXT);
	}

	// 菜单
	UpdateMenu(pWnd->GetMenu());
}

void CLanguageManager::UpdateMenu( CMenu* pMenu )
{
	if (pMenu == NULL)
		return;

	CString strText;
	CMenu* pSubMenu = NULL;
	int nMenuID = 0;
	int nItemCount = pMenu->GetMenuItemCount();

	for (int i=0; i<nItemCount; ++i)
	{
		if (pMenu->GetMenuString(i, strText, MF_BYPOSITION) > 0)
		{
			nMenuID = pMenu->GetMenuItemID(i);
			pMenu->ModifyMenu(i, MF_BYPOSITION, nMenuID, LMIC(strText));
		}

		UpdateMenu(pMenu->GetSubMenu(i));
	}
}
