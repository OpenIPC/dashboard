// ReadWriteExcel.h: interface for the CReadWriteExcel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_READWRITEEXCEL_H__30C57E8E_DC99_4DB1_8689_4B89AD53C223__INCLUDED_)
#define AFX_READWRITEEXCEL_H__30C57E8E_DC99_4DB1_8689_4B89AD53C223__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "excel.h"
class CReadWriteExcel  
{
public:
	CReadWriteExcel();
	virtual ~CReadWriteExcel();
public:
    CString m_strFilePath;

    BOOL    SetFilePath(CString strPath);
    void    SetUseRange(int iSheet);
    CString GetCellString(int iRow,int iCol);
    BOOL    SetCellString(int iRow,int iCol,CString str);
    int     GetRowCount();
    int     GetColCount();

    Range m_usedRange;
private:
    _Application m_excelapp;
    Workbooks m_books;
    _Workbook m_book;
    Worksheets m_sheets;
    _Worksheet m_sheet;
    
    Range m_range;
    LPDISPATCH m_lpDisp;   
};

#endif // !defined(AFX_READWRITEEXCEL_H__30C57E8E_DC99_4DB1_8689_4B89AD53C223__INCLUDED_)
