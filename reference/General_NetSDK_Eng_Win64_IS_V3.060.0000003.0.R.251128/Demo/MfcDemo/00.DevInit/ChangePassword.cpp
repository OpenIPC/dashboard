// ChangePassword.cpp : implementation file
//

#include "stdafx.h"
#include "searchdevice.h"
#include "ChangePassword.h"
#include "LibQREncode/qrencode.h"
#include <iostream>

#pragma warning(disable:4291)
#ifndef NEW
#define NEW new(std::nothrow)
#endif

#define QR_PICTURE_PATH "./QRpicture/QRCode.bmp"
/////////////////////////////////////////////////////////////////////////////
// CChangePassword dialog

CResetPassword::CResetPassword(CWnd* pParent /*=NULL*/)
	: CDialog(CResetPassword::IDD, pParent)
	, m_bOverseas(false)
{
	//{{AFX_DATA_INIT(CChangePassword)
	m_strSecuityCode = _T("");
	m_strNewPwd = _T("");
	m_strConfirmPwd = _T("");
	//}}AFX_DATA_INIT

	memset(m_cQrCode, 0, MAX_QRCODE_LEN);
	m_showPicture = NULL;

	m_showPicture = NEW CShowPicture();
	if(m_showPicture == NULL)
		return;
    strncpy(m_fileName, QR_PICTURE_PATH, sizeof(m_fileName) - 1);

	memset(m_szMac, 0, sizeof(m_szMac));
	memset(m_szUserName, 0, sizeof(m_szUserName));
	m_byInitStaus = 0;
	m_bOverseas = false;
}


void CResetPassword::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangePassword)
	DDX_Text(pDX, IDC_EDIT_SECUTITYCODE, m_strSecuityCode);
	DDX_Text(pDX, IDC_EDIT_NEWPWD, m_strNewPwd);
	DDX_Text(pDX, IDC_EDIT_CONFIRMPWD, m_strConfirmPwd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResetPassword, CDialog)
	//{{AFX_MSG_MAP(CChangePassword)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CResetPassword::OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangePassword message handlers

void CResetPassword::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(TRUE);
	if (m_strSecuityCode.IsEmpty())
	{
		MessageBox(ConvertString("Please input security code"), ConvertString("Prompt"));
		return;
	}
    
    if (m_strNewPwd.IsEmpty())
    {
        MessageBox(ConvertString("Please input password"), ConvertString("Prompt"));
        return ;
    }
    if (m_strConfirmPwd.IsEmpty())
	{
		MessageBox(ConvertString("Please confirm password"), ConvertString("Prompt"));
		return ;
	}
	if(m_strNewPwd.Compare(m_strConfirmPwd))
	{
		MessageBox(ConvertString("Confirm password is invalid,please input again"),ConvertString("Prompt"));
		return ;
	}

	CDialog::OnOK();
}


//Create bmp picture 
bool CResetPassword::MyQRGenerator(char *szSourceSring)
{
    if (NULL == szSourceSring)
    {
        return false;
    }

	unsigned int unWidth;
	unsigned int unWidthAdjusted;
	unsigned int unDataBytes;
	unsigned char* pRGBData;
	unsigned char* pSourceData;
	unsigned char* pDestData;
	QRcode*		pQRC = NULL;
	FILE*		outFile = NULL;

	pQRC = QRcode_encodeString(szSourceSring, 0, QR_ECLEVEL_H, QR_MODE_8, 1);
	if(0 == pQRC)
	{
		return false;
	}
	
	unWidth = pQRC->width;
	unWidthAdjusted = unWidth * OUT_FILE_PIXEL_PRESCALER * 3;
	if (unWidthAdjusted % 4)
	{
		unWidthAdjusted = (unWidthAdjusted / 4 + 1) * 4;
	}
	unDataBytes = unWidthAdjusted * unWidth * OUT_FILE_PIXEL_PRESCALER;

	// Allocate pixels buffer
	if (!(pRGBData = NEW unsigned char[unDataBytes]))
	{
		return false;
	}

	// Preset to white
	memset(pRGBData, 0xff, unDataBytes);

	// Prepare bmp headers
	BITMAPFILEHEADER kFileHeader;
	kFileHeader.bfType = 0x4d42;  // "BM"
	kFileHeader.bfSize =	sizeof(BITMAPFILEHEADER) +sizeof(BITMAPINFOHEADER) +unDataBytes;
	kFileHeader.bfReserved1 = 0;
	kFileHeader.bfReserved2 = 0;
	kFileHeader.bfOffBits =	sizeof(BITMAPFILEHEADER) +sizeof(BITMAPINFOHEADER);
	BITMAPINFOHEADER kInfoHeader;
	kInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	kInfoHeader.biWidth = unWidth * OUT_FILE_PIXEL_PRESCALER;
	kInfoHeader.biHeight = -((int)unWidth * OUT_FILE_PIXEL_PRESCALER);
	kInfoHeader.biPlanes = 1;
	kInfoHeader.biBitCount = 24;
	kInfoHeader.biCompression = BI_RGB;
	kInfoHeader.biSizeImage = 0;
	kInfoHeader.biXPelsPerMeter = 0;
	kInfoHeader.biYPelsPerMeter = 0;
	kInfoHeader.biClrUsed = 0;
	kInfoHeader.biClrImportant = 0;

	// Convert QrCode bits to bmp pixels
	pSourceData = pQRC->data;
	int i, j, k, l;
	for(i = 0; i < unWidth; i++)
	{
		pDestData = pRGBData + unWidthAdjusted * i * OUT_FILE_PIXEL_PRESCALER;
		for(j = 0; j < unWidth; j++)
		{
			if (*pSourceData & 1)
			{
				for(k = 0; k < OUT_FILE_PIXEL_PRESCALER; k++)
				{
					for(l = 0; l < OUT_FILE_PIXEL_PRESCALER; l++)
					{
						*(pDestData + l * 3 + unWidthAdjusted * k) = PIXEL_COLOR_B;
						*(pDestData + 1 + l * 3 + unWidthAdjusted * k) = PIXEL_COLOR_G;
						*(pDestData + 2 + l * 3 + unWidthAdjusted * k) = PIXEL_COLOR_R;
					}
				}
			}
			pDestData += 3 * OUT_FILE_PIXEL_PRESCALER;
			pSourceData++;
		}
	}


	// Output the bmp file
	outFile=fopen(m_fileName, "wb");
	if(NULL == outFile)
	{
		delete[] pRGBData;
		return false;
	}

	fwrite(&kFileHeader, sizeof(BITMAPFILEHEADER), 1, outFile);
	fwrite(&kInfoHeader, sizeof(BITMAPINFOHEADER), 1, outFile);
	fwrite(pRGBData, sizeof(unsigned char), unDataBytes, outFile);
	fclose(outFile);

	delete []pRGBData;
	pRGBData = NULL;
	QRcode_free(pQRC);

    return true;
}

BOOL CResetPassword::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here	
   
    if(m_bOverseas)
    {
        // Oversea device
        GetDlgItem(IDC_STATIC1)->SetWindowText(ConvertString("Please get security code as follows:"));
        GetDlgItem(IDC_STATIC2)->SetWindowText(ConvertString("1. Please download Easy4ip app and then scan the QR-code from [Me]-[Reset device password]"));
        GetDlgItem(IDC_STATIC3)->SetWindowText(ConvertString("2. Scan the QR-code and then send the result information to support_rpwd@global.dahuatech.com"));
        GetDlgItem(IDC_STATIC4)->SetWindowText(ConvertString(""));
    }
    else
    {
     // Mainland Device
        GetDlgItem(IDC_STATIC1)->SetWindowText(ConvertString("Please get security code as follows:"));
        GetDlgItem(IDC_STATIC2)->SetWindowText(ConvertString("1. Please flow the WeChat number [Zhejiang Dahua Customer Service]-[Password Reset] scan the QR-code"));
        GetDlgItem(IDC_STATIC3)->SetWindowText(ConvertString("2. If you use other methods to scan the QR-code, please send the QR-code information to Mobile/Telecom:10690669121821, Unicom:10690067121821"));
		GetDlgItem(IDC_STATIC4)->SetWindowText(ConvertString(""));
    }
    g_SetWndStaticText(this);

	// 首先判断工作目录下是否存在 QR_PICTURE_PATH 中的 QRpicture 目录，如果不存在，则创建
	WIN32_FIND_DATA wfd;
	char szQrDir[] = "./QRpicture";
	HANDLE hFind = FindFirstFile(szQrDir, &wfd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		// 不存在该路径，创建该路径
		BOOL bRet = CreateDirectory(szQrDir, NULL);
		if (FALSE == bRet)
		{
			MessageBox(ConvertString("Fail to create QR Directory"),ConvertString("Prompt"));
		}
	}

	FindClose(hFind);
    ShowQrCode();
	return TRUE;
}

void CResetPassword::OnCancel() 
{
	// TODO: Add extra cleanup here
	CDialog::OnCancel();
}

BOOL CResetPassword::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class

	if(m_showPicture != NULL)
	{
		delete m_showPicture;
		m_showPicture = NULL;
	}
	
	return CDialog::DestroyWindow();
}

void CResetPassword::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	m_showPicture->RepaintPictures();
}


int CResetPassword::ShowQrCode()
{
    bool bRet = MyQRGenerator(m_cQrCode);
    if (false == bRet)
    {
        MessageBox(ConvertString("Fail to create QRcode"),ConvertString("Prompt"));
        return -1;
    }
    
    HWND pictureHwd = GetDlgItem(IDC_STATIC_QR)->GetSafeHwnd();
    if (NULL == pictureHwd)
    {
        return -1;
    }
	m_showPicture->AddPicture(pictureHwd, m_fileName,0,0);
    return  0;
}

void CResetPassword::OnBnClickedOk()
{
	// TODO
    UpdateData();
	if (m_strSecuityCode.IsEmpty())
	{
		MessageBox(ConvertString("Please input security code"), ConvertString("Prompt"));
		return;
	}

	if (m_strNewPwd.IsEmpty())  
	{
		MessageBox(ConvertString("Please input password"), ConvertString("Prompt"));
		return;
	}

    if (m_strConfirmPwd.IsEmpty())
    {
        MessageBox(ConvertString("Please confirm password"), ConvertString("Prompt"));
        return;
    }

	if (m_strConfirmPwd != m_strNewPwd)
	{
		MessageBox(ConvertString("Confirm password is invalid,please input again"), ConvertString("Prompt"));
		return;
	}
	
	ResetPwd();
	OnOK();
}

int CResetPassword::ResetPwd()
{
	char* szNewPwd			= m_strNewPwd.GetBuffer(0);
	char* szSecurityCode	= m_strSecuityCode.GetBuffer(0);

	NET_IN_RESET_PWD ResetPwdIn = {0};
	NET_OUT_RESET_PWD pResetPwdOut = {0};
	ResetPwdIn.dwSize = sizeof(ResetPwdIn);
	pResetPwdOut.dwSize = sizeof(pResetPwdOut);	

	ResetPwdIn.byInitStaus = m_byInitStaus;
	// mac address
	strncpy(ResetPwdIn.szMac, m_szMac, sizeof(ResetPwdIn.szMac) - 1);
	// username
	strncpy(ResetPwdIn.szUserName, m_szUserName, sizeof(ResetPwdIn.szUserName) - 1);  
	// password 
	strncpy(ResetPwdIn.szPwd,  szNewPwd, sizeof(ResetPwdIn.szPwd) - 1);   	
	// security code
	strncpy(ResetPwdIn.szSecurity, szSecurityCode, sizeof(ResetPwdIn.szSecurity) - 1);    

	m_strNewPwd.ReleaseBuffer();
	m_strSecuityCode.ReleaseBuffer();

	//Find Password
	BOOL bRet = CLIENT_ResetPwd(&ResetPwdIn, &pResetPwdOut, 3000, NULL);	 
	if(0 == bRet)
	{
		CString strTmp;
		strTmp.Format("%s:%d", ConvertString("Password Reset Failed with Error"), CLIENT_GetLastError() & 0x7fffffff);
		MessageBox(strTmp, ConvertString("Prompt"));
		return -1;
	}
	MessageBox(ConvertString("Password Reset Success"),ConvertString("Prompt"));

	return 0;
}


void CResetPassword::SetResetPwdParam(char* szMac, unsigned int nMacLen, char* szUserName, unsigned int nNameLen, BYTE byInitStaus)
{
	if (NULL == szMac || NULL == szUserName)
	{
		return;
	}
	
	strncpy(m_szMac, szMac, sizeof(m_szMac)-1);
	strncpy(m_szUserName, szUserName, sizeof(m_szUserName)-1);

	m_byInitStaus = byInitStaus;
}

void CResetPassword::SetQRCodeScanDescription(bool bOverseas)
{
	m_bOverseas = bOverseas;	
}


BOOL CResetPassword::PreTranslateMessage(MSG* pMsg) 
{
    // TODO: Add your specialized code here and/or call the base class
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
    {
        return TRUE;
    }

    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
    {
        return TRUE;
    }

    return CDialog::PreTranslateMessage(pMsg);
}
