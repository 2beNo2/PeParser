
// PeparserDlg.h: 头文件
//

#pragma once
#include "CMyPe.h"

// CPeparserDlg 对话框
class CPeparserDlg : public CDialogEx
{
// 构造
public:
	CPeparserDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PEPARSER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	// 树控件，显示PE文件的总体结构
	CTreeCtrl m_TreeCtrl;
	void InitTreeCtrl();
	void ExpandTreeCtrl(HTREEITEM hItem);

private:
	// 在ListCtrl控件上显示对应的PE结构体的成员
	CListCtrl m_MainListCtrl;
	void ClearListCtrl();
	void ShowDosHeader();
	void ShowNtHeader();
	void ShowFileHeader();
	void ShowOptionalHeader();
	void ShowSectionHeader();

	void ShowExportDirectory();
	void ShowImportDirectory();
	void ShowResourceDirectory();
	void ShowRelocDirectory();
	void ShowTlsDirectory();

private:
	// PE解析
	CString m_csFileName;
	CString m_csFilePath;
	CMyPe* m_pMyPe;
	void InitHeapData();
	void FinitHeapData();

public:
	// 消息响应
	afx_msg void OnOpenfile();
	afx_msg void OnClose();
	afx_msg void OnSelchangedTreePe(NMHDR* pNMHDR, LRESULT* pResult);

};
