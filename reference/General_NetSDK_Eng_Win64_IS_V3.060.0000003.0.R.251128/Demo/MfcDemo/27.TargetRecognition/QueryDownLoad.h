#pragma once
#include "afxcmn.h"
#include "afxwin.h"


#define		FILEPATH_LEN 128
//the enum of the Query Type
enum emQueryType
{
	QUERY_VIDEO,		//Video query
	QUERY_PICFD,		//Face Detection query
	QUERY_PICFR			//target Recognition query
};

typedef struct
{
	//Video Face Detection param
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

	EM_DEV_EVENT_FACEDETECT_SEX_TYPE				emSex;	
	EM_FACEDETECT_GLASSES_TYPE						emGlasses;
	EM_MASK_STATE_TYPE								emMask;
	EM_BEARD_STATE_TYPE								emBeard;
	EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE			emEmotion;

} STR_RECORDFILE_INFO;

class CQueryDownLoad : public CDialog
{
	DECLARE_DYNAMIC(CQueryDownLoad)

public:
	CQueryDownLoad( int nChannelNum, LLONG lLoginid, char *pSoftPath = NULL, CWnd* pParent = NULL);   
	virtual ~CQueryDownLoad();
	virtual BOOL OnInitDialog();

	enum { IDD = IDD_DOWNLOAD_RECOGNITION_DIALOG };
	void			ReceivePlayPos(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    

	DECLARE_MESSAGE_MAP()

private:
	afx_msg void OnCbnSelchangeComboFileType();
	afx_msg void OnBnClickedButtonQueryPrepage();
	afx_msg void OnBnClickedButtonQueryDownload();
	afx_msg void OnBnClickedButtonQueryNextpage();
	afx_msg void OnBnClickedButtonQueryStart();
	afx_msg void OnLvnItemchangedListQueryResult(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnDoDownLoad(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDoPlayBack(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedCheckAttributeEnable();

private:
	void	InitControl();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL	FindFaceDetectionPicture();		//Queyt picture of the FaceDetection event
	BOOL	FindTargetRecognitionPicture();	//Queyt picture of the TargetRecognition event

	BOOL	FindFaceEventPicture(DWORD dwEventType);		//Queyt picture of the face event
	BOOL	FindFaceVideo();				//Query video of the Face event

	int		FindNextFaceVideo(int nSearchCount);
	int		FindNextFaceDetectionPicture(int nSearchCount);
	int		FindNextTargetRecognitionPicure(int nSearchCount);

	int		FindNextFaceEventType(int nSearchCount);

	BOOL	DownLoadFaceVideo(int nSel);		 	 //DownLoad video of the Face event
	BOOL	DownLoadFaceDetectionPicture(int nSel); //DownLoad picture of the FaceDetection event
	BOOL	DownLoadTargetRecognitionPicure( int nSel);//DownLoad picture of the TargetRecognition event

	void	ClosePlayBackHandle();			//Close the handle of the PlayBack
	void	CloseFindHandle();				//Close the handle of the Search
	void	CloseDownLoadHandle();			//Close the handle of the Download

	//void	DoFindNextPage();
	int		DoFindNextPageEx(int nSearchCount);
	void	ClearQueryResultInfoVector();				
	void	UpdateLcQueryList( int nInfoNum);			//Refresh Control List
	emQueryType	getCurrentQueryType();
	void	OnTimer(UINT_PTR nIDEvent);
	void	GetTimeFromUI();
	BOOL	GetAttributeFromUI();
	void	ClearAttribute();

	void	SetAttributeInfo(std::string& strAttributeInfo, STR_RECORDFILE_INFO strRecordInfo);

private:
	CProgressCtrl m_PlayBackProcess;
	CProgressCtrl m_ProcessDownLoad;
	CListCtrl	m_lcQueryResult;
	CComboBox	m_comboFileType;
	CComboBox	m_comboQueryChannel;
	CComboBox	m_comboEventType;
	CComboBox	m_comboSex;
	CComboBox	m_comboGlasses;
	CComboBox	m_comboMask;
	CComboBox	m_comboBeard;
	CButton m_AttributeEnable;
	CComboBox m_comboEmotion;

private:
	int			m_nChannelNum;
	LLONG		m_lLogin;
	LLONG		m_lFindHandle;
	LLONG		m_lDownLoadHandle;
	LLONG		m_playBackHandle;

	UINT		m_nTimer;
	DWORD		m_dwPlayBackTotalSize;
	DWORD		m_dwPlayBackCurValue;

	DWORD		m_dwDownLoadTotalSize;
	DWORD		m_dwDownLoadCurValue;
	char 		*m_pSoftPath;

	EM_DEV_EVENT_FACEDETECT_SEX_TYPE	m_emSex;	
	EM_FACEDETECT_GLASSES_TYPE			m_emGlasses;
	EM_MASK_STATE_TYPE					m_emMask;
	EM_BEARD_STATE_TYPE					m_emBeard;
	EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE	m_emEmotion;
	int									m_nAge[2];
	int									m_nSimilaryRange[2];

	std::vector<STR_RECORDFILE_INFO*>	m_QueryResultInfoVecotr;		
	int			m_nPage;						
	int			m_nInfoCount;					//Query Result Count
	emQueryType	m_emQuryType;					//Current Query type

	int			m_nQueryType;					// 0:event 1:attribute
	int			m_nAttributeFlag;				// 0: not check  1:check

	NET_TIME	m_DateStart;
	NET_TIME	m_DateEnd;
	
public:
	afx_msg void OnCbnSelchangeComboAlarmType();
};
