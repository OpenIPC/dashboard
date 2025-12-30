// CardManager.cpp : 实现文件
//

#include "stdafx.h"
#include "VTODemo.h"
//#include "CardManager.h"


// CCardManager 对话框

IMPLEMENT_DYNAMIC(CCardManager, CDialog)

CCardManager::CCardManager(CWnd* pParent /*=NULL*/)
	: CDialog(CCardManager::IDD, pParent)
{

}

CCardManager::~CCardManager()
{
}

void CCardManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCardManager, CDialog)
END_MESSAGE_MAP()


// CCardManager 消息处理程序
