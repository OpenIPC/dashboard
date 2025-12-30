#pragma once
#include "afxcmn.h"
#include "afxwin.h"

typedef struct
{
	unsigned int        ch;                         // channel no.
	char                filename[124];              // File name
	unsigned int        framenum;                   // the total number of file frames
	unsigned int        size;                       // File length, unit: Kbyte 
	NET_TIME            starttime;                  // start time
	NET_TIME            endtime;                    // end time
	unsigned int        driveno;                    // HDD number 
	unsigned int        startcluster;               // Initial cluster number 
	BYTE                nRecordFileType;            // Recorded file type  0:general record;1:alarm record ;2:motion detection;3:card number record ;4:image ; 19:Pos record ;255:all
	BYTE                bImportantRecID;            // 0:general record 1:Important record
	BYTE                bHint;                      // Document Indexing
	BYTE                bRecType;                   // 0-main stream record 1-sub1 stream record 2-sub2 stream record 3-sub3 stream record
} STR_RECORDFILE_INFO;

class CQueryDownload : public CDialog
{
	DECLARE_DYNAMIC(CQueryDownload)

public:
	CQueryDownload(int nChannelNum, LLONG lLoginid, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CQueryDownload();

// 对话框数据
	enum { IDD = IDD_QUERY_DOWNLOAD_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonQueryPrepage();
	afx_msg void OnBnClickedButtonQueryNextpage();
	afx_msg void OnLvnItemchangedListQueryResult(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnDoDownLoad(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonQueryStart();
	afx_msg void OnBnClickedButtonQueryDownload();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

private:
	void		InitClQueryResult();
	BOOL		DoFindFileEx(int nChannel, int nMediaType, DWORD dwEventType);
	int			DoFindNextFileEx();
	void		OnDoDownload(int nSel);
	void		CloseFindHandle();
	void		CloseDownLoadHandle();
	void		ClosePlayBackHandle();
	void		ClearFileInfoVector();
	void		UpdateLcQueryList( int nInfoNum);			//Refresh Control List
	void		DealDoFindResultInfo(NET_OUT_MEDIA_QUERY_FILE *pMediaFileInfo, int nCount);
	void		ReceivePlayPos(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize);

	void		SetData(NET_TIME &stNetTime, int nID);
	void		SetTime(NET_TIME &stNetTime, int nID);

private:
	LLONG		m_lLoginid;
	LLONG		m_lFindHandle;
	LLONG		m_lDownLoadHandle;
	LLONG		m_playBackHandle;
	int			m_nChannelNum;
	int			m_dwPlayBackTotalSize;
	int			m_dwCurPlayBackSize;
	int			m_dwDownLoadTotalSize;
	int			m_dwCurDownLoadSize;
	int			m_nPage;
	int			m_nInfoCount;

	int			m_QueryFileType;	//1: picture 2: video
	DWORD		m_dwQueryEventType;	

	NET_TIME	m_DateStart;
	NET_TIME	m_DateEnd;
	
	std::vector<STR_RECORDFILE_INFO*> m_DownLoadInfoVector;

	char		*m_pszSoftPath;

public:
	CListCtrl m_lcQueryResult;
	CProgressCtrl m_PlayBackProcess;
	CProgressCtrl m_DownLoadProcess;
	CComboBox m_comboFileType;
	CComboBox m_comboQueryChannel;
	CComboBox m_comboEventType;
};
