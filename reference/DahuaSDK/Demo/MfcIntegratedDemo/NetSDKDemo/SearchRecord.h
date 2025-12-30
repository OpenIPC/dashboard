#if !defined(AFX_SEARCHRECORD_H__04200139_FCAC_487E_908D_E2E8A04BFD0D__INCLUDED_)
#define AFX_SEARCHRECORD_H__04200139_FCAC_487E_908D_E2E8A04BFD0D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SearchRecord.h : header file
//
#include "searchrecord.h"
/////////////////////////////////////////////////////////////////////////////
// CSearchRecord dialog

typedef struct _FileInfoNode {
	NET_RECORDFILE_INFO fileinfo;
	DeviceNode *pDevice;
}FileInfoNode;

class CSearchRecord : public CDialog
{
	LLONG m_pdownloadfile;
	DWORD m_downloadtotal;
	DWORD m_downloadsize;
	
	void AddFileListInfo( NET_RECORDFILE_INFO *pfileinfo, int iMaxNum);
	void UpdataResultList();
// Construction
public:
	FileInfoNode *m_playfile;
	FileInfoNode m_playList[DH_MAX_CHANNUM];
	int			 m_playCount;

	void UpdateDownloadPos(LLONG iHandle, DWORD dwTotal, DWORD dwDownload);
	
	CSearchRecord(CWnd* pParent = NULL);   // standard constructor

	//Â¼Ïó²éÑ¯¸¨Öúº¯Êý
	void AddOneFileToList(NET_RECORDFILE_INFO *pfileinfo, int index);
// Dialog Data
	//{{AFX_DATA(CSearchRecord)
	enum { IDD = IDD_SEARCH_RECORD };
	CListCtrl	m_resultlist;
	CProgressCtrl	m_download_pos;
	CComboBox	m_sch_type;
	CComboBox	m_devicesel;
	CString	m_cardinput;
	int		m_channelsel;
	CTime	m_enddate;
	CTime	m_endtime;
	CTime	m_startdate;
	CTime	m_starttime;
	//}}AFX_DATA
	friend void CALLBACK DownLoadPos(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, LDWORD dwUser);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSearchRecord)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


public:

	int InitComboxCallback_Imp(const DeviceNode& node);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSearchRecord)
	afx_msg void OnSchStartsearch();
	virtual BOOL OnInitDialog();
	afx_msg void OnSchDownload();
	afx_msg void OnClose();
	afx_msg void OnPlayback();
	afx_msg void OnSchClearList();
	afx_msg void OnDblclkSchResultlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddToList();
	afx_msg void OnSelchangeSchType();
	afx_msg LRESULT OnUpdateDownLoadPos(WPARAM wparam, LPARAM lparam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CFileListProtector
{
public:
	CFileListProtector(CPtrList **list):m_ptrfilelist(list)
	{
		*m_ptrfilelist = new CPtrList(sizeof(FileInfoNode));
		*list = *m_ptrfilelist;
	}
	~CFileListProtector()
	{
		int count = (*m_ptrfilelist)->GetCount();
		for (int i = 0; i < count; i++)
		{
			delete (FileInfoNode *)((*m_ptrfilelist)->GetTail());
			(*m_ptrfilelist)->RemoveTail();
		}
		(*m_ptrfilelist)->RemoveAll();
		delete (*m_ptrfilelist);
	}
private:
	CPtrList **m_ptrfilelist;
};

extern CPtrList  *g_ptrfilelist;
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEARCHRECORD_H__04200139_FCAC_487E_908D_E2E8A04BFD0D__INCLUDED_)
