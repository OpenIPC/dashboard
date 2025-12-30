#pragma once

#define LANGUAGE_EN			0
#define LANGUAGE_ZHCN		1

#define LMI				CLanguageManager::Instance()
#define LMIC(text)		CLanguageManager::Instance().ConvertString(text)
#define LMIU(pWnd)		CLanguageManager::Instance().UpdateWindowString(pWnd)

class CLanguageManager
{
public:
	CLanguageManager(void);
	~CLanguageManager(void);

public:
	static CLanguageManager& Instance();

	void SetLanguage(int nLanguage);
	CString ConvertString(LPCTSTR lpszText);
	void UpdateWindowString(CWnd* pWnd);

private:
	void UpdateMenu(CMenu* pMenu);

private:
	CLanguageManager(const CLanguageManager&);
	CLanguageManager& operator=(const CLanguageManager&);

protected:
	int m_nLanguage;
	TCHAR m_szFilePath[_MAX_PATH];
};
