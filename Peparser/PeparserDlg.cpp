
// PeparserDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Peparser.h"
#include "PeparserDlg.h"
#include "afxdialogex.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPeparserDlg 对话框



CPeparserDlg::CPeparserDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PEPARSER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	InitHeapData();
}

void CPeparserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, TREE_PE, m_TreeCtrl);
	DDX_Control(pDX, LST_PE, m_MainListCtrl);
}

BEGIN_MESSAGE_MAP(CPeparserDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(MN_OPENFILE, &CPeparserDlg::OnOpenfile)
	ON_WM_CLOSE()
	ON_NOTIFY(TVN_SELCHANGED, TREE_PE, &CPeparserDlg::OnSelchangedTreePe)
END_MESSAGE_MAP()


// CPeparserDlg 消息处理程序

BOOL CPeparserDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	// 加载文件菜单
	CMenu menuFile;
	menuFile.LoadMenu(MN_FILE);
	this->SetMenu(&menuFile);

	// 初始化树控件
	InitTreeCtrl();

	// 设置ListCtrl额外样式：网格线和整行选中
	m_MainListCtrl.SetExtendedStyle(
		m_MainListCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// 初始化double的ListCtrl
	InitDoubleListCtrl();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CPeparserDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPeparserDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPeparserDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPeparserDlg::InitTreeCtrl()
{
	m_TreeCtrl.InsertItem(TEXT("Dos Header"));
	HTREEITEM  hItem = m_TreeCtrl.InsertItem(TEXT("Nt Header"));
	m_TreeCtrl.InsertItem(TEXT("File Header"), hItem);
	hItem = m_TreeCtrl.InsertItem(TEXT("Optional Header"), hItem);
	m_TreeCtrl.InsertItem(TEXT("Data Directories"), hItem);
	m_TreeCtrl.InsertItem(TEXT("Section Header"));
	m_TreeCtrl.InsertItem(TEXT("Export Directory"));
	m_TreeCtrl.InsertItem(TEXT("Import Directory"));
	m_TreeCtrl.InsertItem(TEXT("Resource Directory"));
	m_TreeCtrl.InsertItem(TEXT("Relocation Directory"));
	m_TreeCtrl.InsertItem(TEXT("TLS Directory"));

	ExpandTreeCtrl(m_TreeCtrl.GetRootItem());
}

void CPeparserDlg::ExpandTreeCtrl(HTREEITEM hItem)
{
	if (!m_TreeCtrl.ItemHasChildren(hItem))//如果树控件根节点没有子节点则返回
	{
		hItem = m_TreeCtrl.GetNextSiblingItem(hItem);
		if (hItem == NULL)
		{
			return;
		}
	}
	HTREEITEM hNextItem = m_TreeCtrl.GetChildItem(hItem);//若树控件的根节点有子节点则获取根节点的子节点
	while (hNextItem != NULL)//若有
	{
		ExpandTreeCtrl(hNextItem);//递归，展开子节点下的所有子节点
		hNextItem = m_TreeCtrl.GetNextItem(hNextItem, TVGN_NEXT);//获取根节点的下一个子节点
	}
	m_TreeCtrl.Expand(hItem, TVE_EXPAND);
}

void CPeparserDlg::OnOpenfile()
{
	// TODO: 在此添加命令处理程序代码
	CFileDialog FileDialog(TRUE);
	int nRet = FileDialog.DoModal();
	if(nRet == IDOK)
	{
		m_csFileName = FileDialog.GetFileName();
		m_csFilePath = FileDialog.GetPathName();

		int uIsPe = CMyPe::IsPeFile(m_csFilePath.GetBuffer());
		switch (uIsPe)
		{
		case CMyPe::FIlE_OPENFAILD:
		{
			AfxMessageBox(TEXT("文件打开失败！"));
			return;
		}
		case CMyPe::FILE_NOTPE:
		{
			AfxMessageBox(TEXT("文件不是PE格式！"));
			return;
		}
		}

		// 程序是PE格式
		m_pMyPe = new CMyPe(m_csFilePath.GetBuffer());

		// 修改树控件的焦点
		HTREEITEM hTreeItem = m_TreeCtrl.GetRootItem();
		m_TreeCtrl.SelectItem(hTreeItem);
		ShowDosHeader();
	}
}

void CPeparserDlg::InitHeapData()
{
	m_pMyPe = NULL;
	m_DoubleAListCtrl = NULL;
	m_DoubleBListCtrl = NULL;
}

void CPeparserDlg::FinitHeapData()
{
	if (m_pMyPe != NULL)
	{
		delete m_pMyPe;
		m_pMyPe = NULL;
	}

	if (m_DoubleAListCtrl != NULL)
	{
		delete m_DoubleAListCtrl;
		m_DoubleAListCtrl = NULL;
	}

	if (m_DoubleBListCtrl != NULL)
	{
		delete m_DoubleBListCtrl;
		m_DoubleBListCtrl = NULL;
	}
}

void CPeparserDlg::ClearMainListCtrl()
{
	m_MainListCtrl.ShowWindow(SW_SHOW);
	m_DoubleAListCtrl->ShowWindow(SW_HIDE);
	m_DoubleBListCtrl->ShowWindow(SW_HIDE);

	// 删除所有的item
	m_MainListCtrl.DeleteAllItems();

	// 删除所有的表头
	int nColumnCount = m_MainListCtrl.GetHeaderCtrl()->GetItemCount();
	for (int i = 0; i < nColumnCount; i++)
	{
		m_MainListCtrl.DeleteColumn(0);
	}
}

void CPeparserDlg::ClearDoubleAListCtrl()
{
	m_MainListCtrl.ShowWindow(SW_HIDE);
	m_DoubleAListCtrl->ShowWindow(SW_SHOW);
	m_DoubleBListCtrl->ShowWindow(SW_SHOW);

	// 删除所有的item
	m_DoubleAListCtrl->DeleteAllItems();

	// 删除所有的表头
	int nColumnCount = m_DoubleAListCtrl->GetHeaderCtrl()->GetItemCount();
	for (int i = 0; i < nColumnCount; i++)
	{
		m_DoubleAListCtrl->DeleteColumn(0);
	}
}

void CPeparserDlg::ClearDoubleBListCtrl()
{
	m_MainListCtrl.ShowWindow(SW_HIDE);
	m_DoubleAListCtrl->ShowWindow(SW_SHOW);
	m_DoubleBListCtrl->ShowWindow(SW_SHOW);

	// 删除所有的item
	m_DoubleBListCtrl->DeleteAllItems();

	// 删除所有的表头
	int nColumnCount = m_DoubleBListCtrl->GetHeaderCtrl()->GetItemCount();
	for (int i = 0; i < nColumnCount; i++)
	{
		m_DoubleBListCtrl->DeleteColumn(0);
	}
}

void CPeparserDlg::InitDoubleListCtrl()
{
	// 获取MainListCtrl在客户区的rect
	CRect mainListRect;
	GetDlgItem(LST_PE)->GetWindowRect(&mainListRect);
	ScreenToClient(&mainListRect);

	// 初始化新的ListCtrl的rect
	CRect doubleAListRect;
	doubleAListRect.top = mainListRect.top;
	doubleAListRect.left = mainListRect.left;
	doubleAListRect.bottom = (mainListRect.bottom - mainListRect.top) / 2 + mainListRect.top;
	doubleAListRect.right = mainListRect.right;
	m_DoubleAListCtrl = new CListCtrl;
	m_DoubleAListCtrl->Create(GetWindowLong(m_MainListCtrl.m_hWnd, GWL_STYLE),
								doubleAListRect,
								this,
								0x300);
	m_DoubleAListCtrl->SetExtendedStyle(
		m_DoubleAListCtrl->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_DoubleAListCtrl->ShowWindow(SW_HIDE);


	CRect doubleBListRect;
	doubleBListRect.top = (mainListRect.bottom - mainListRect.top) / 2  + mainListRect.top;
	doubleBListRect.left = mainListRect.left;
	doubleBListRect.bottom = mainListRect.bottom;
	doubleBListRect.right = mainListRect.right;
	m_DoubleBListCtrl = new CListCtrl;
	m_DoubleBListCtrl->Create(GetWindowLong(m_MainListCtrl.m_hWnd, GWL_STYLE),
								doubleBListRect,
								this,
								0x301);
	 
	m_DoubleBListCtrl->SetExtendedStyle(
		m_DoubleBListCtrl->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_DoubleBListCtrl->ShowWindow(SW_HIDE);
}

void CPeparserDlg::ShowDosHeader()
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)m_pMyPe->GetDosHeaderPointer();

	ClearMainListCtrl();
	// 插入表头
	m_MainListCtrl.InsertColumn(0, TEXT("Member"), LVCFMT_LEFT, 150);
	m_MainListCtrl.InsertColumn(1, TEXT("Offset"), LVCFMT_LEFT, 120);
	m_MainListCtrl.InsertColumn(2, TEXT("Size"), LVCFMT_LEFT, 120);
	m_MainListCtrl.InsertColumn(3, TEXT("Value"), LVCFMT_LEFT, 120);

	// 插入表项
	int nItem = 0;
	CString csTmp;
	csTmp.Format(TEXT("%04X"), pDosHeader->e_magic);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_magic"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000000"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_cblp);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_cblp"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000002"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_cp);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_cp"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000004"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_crlc);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_crlc"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000006"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_cparhdr);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_cparhdr"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000008"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_minalloc);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_minalloc"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("0000000A"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_maxalloc);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_maxalloc"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("0000000C"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_ss);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_ss"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("0000000E"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_sp);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_sp"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000010"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_csum);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_csum"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000012"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_ip);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_ip"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000014"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_cs);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_cs"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000016"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_lfarlc);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_lfarlc"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000018"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_ovno);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_ovno"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("0000001A"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_res[0]);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_res"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("0000001C"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_res[1]);
	m_MainListCtrl.InsertItem(nItem, TEXT(" "));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("0000001E"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;
	csTmp.Format(TEXT("%04X"), pDosHeader->e_res[2]);
	m_MainListCtrl.InsertItem(nItem, TEXT(" "));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000020"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;
	csTmp.Format(TEXT("%04X"), pDosHeader->e_res[3]);
	m_MainListCtrl.InsertItem(nItem, TEXT(" "));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000022"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;


	csTmp.Format(TEXT("%04X"), pDosHeader->e_oemid);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_oemid"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000024"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_oeminfo);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_oeminfo"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000026"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%04X"), pDosHeader->e_res2[0]);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_res2"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000028"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;
	csTmp.Format(TEXT("%04X"), pDosHeader->e_res2[1]);
	m_MainListCtrl.InsertItem(nItem, TEXT(" "));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("0000002A"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;
	csTmp.Format(TEXT("%04X"), pDosHeader->e_res2[2]);
	m_MainListCtrl.InsertItem(nItem, TEXT(" "));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("0000002C"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;
	csTmp.Format(TEXT("%04X"), pDosHeader->e_res2[3]);
	m_MainListCtrl.InsertItem(nItem, TEXT(" "));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("0000002E"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;
	csTmp.Format(TEXT("%04X"), pDosHeader->e_res2[4]);
	m_MainListCtrl.InsertItem(nItem, TEXT(" "));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000030"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;
	csTmp.Format(TEXT("%04X"), pDosHeader->e_res2[5]);
	m_MainListCtrl.InsertItem(nItem, TEXT(" "));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000032"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;
	csTmp.Format(TEXT("%04X"), pDosHeader->e_res2[6]);
	m_MainListCtrl.InsertItem(nItem, TEXT(" "));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000034"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;
	csTmp.Format(TEXT("%04X"), pDosHeader->e_res2[7]);
	m_MainListCtrl.InsertItem(nItem, TEXT(" "));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000036"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;
	csTmp.Format(TEXT("%04X"), pDosHeader->e_res2[8]);
	m_MainListCtrl.InsertItem(nItem, TEXT(" "));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("00000038"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;
	csTmp.Format(TEXT("%04X"), pDosHeader->e_res2[9]);
	m_MainListCtrl.InsertItem(nItem, TEXT(" "));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("0000003A"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew);
	m_MainListCtrl.InsertItem(nItem, TEXT("e_lfanew"));
	m_MainListCtrl.SetItemText(nItem, 1, TEXT("0000003C"));
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
}

void CPeparserDlg::ShowNtHeader()
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)m_pMyPe->GetDosHeaderPointer();
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)m_pMyPe->GetNtHeaderPointer();

	ClearMainListCtrl();
	m_MainListCtrl.InsertColumn(0, TEXT("Member"), LVCFMT_LEFT, 150);
	m_MainListCtrl.InsertColumn(1, TEXT("Offset"), LVCFMT_LEFT, 120);
	m_MainListCtrl.InsertColumn(2, TEXT("Size"), LVCFMT_LEFT, 120);
	m_MainListCtrl.InsertColumn(3, TEXT("Value"), LVCFMT_LEFT, 120);

	int nItem = 0;
	CString csTmp;

	m_MainListCtrl.InsertItem(nItem, TEXT("Signature"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pNtHeaders->Signature);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
}

void CPeparserDlg::ShowFileHeader()
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)m_pMyPe->GetDosHeaderPointer();
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)m_pMyPe->GetNtHeaderPointer();
	PIMAGE_FILE_HEADER pFileHeaders = (PIMAGE_FILE_HEADER)m_pMyPe->GetFileHeaderPointer();

	ClearMainListCtrl();
	m_MainListCtrl.InsertColumn(0, TEXT("Member"), LVCFMT_LEFT, 220);
	m_MainListCtrl.InsertColumn(1, TEXT("Offset"), LVCFMT_LEFT, 120);
	m_MainListCtrl.InsertColumn(2, TEXT("Size"), LVCFMT_LEFT, 120);
	m_MainListCtrl.InsertColumn(3, TEXT("Value"), LVCFMT_LEFT, 120);

	int nItem = 0;
	CString csTmp;

	m_MainListCtrl.InsertItem(nItem, TEXT("Machine"));
	csTmp.Format(TEXT("%04X"), pDosHeader->e_lfanew + 4);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pFileHeaders->Machine);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("NumberOfSections"));
	csTmp.Format(TEXT("%04X"), pDosHeader->e_lfanew + 6);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pFileHeaders->NumberOfSections);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("TimeDateStamp"));
	csTmp.Format(TEXT("%04X"), pDosHeader->e_lfanew + 8);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pFileHeaders->TimeDateStamp);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("PointerToSymbolTable"));
	csTmp.Format(TEXT("%04X"), pDosHeader->e_lfanew + 12);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pFileHeaders->PointerToSymbolTable);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("NumberOfSymbols"));
	csTmp.Format(TEXT("%04X"), pDosHeader->e_lfanew + 16);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pFileHeaders->NumberOfSymbols);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("SizeOfOptionalHeader"));
	csTmp.Format(TEXT("%04X"), pDosHeader->e_lfanew + 20);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pFileHeaders->SizeOfOptionalHeader);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("Characteristics"));
	csTmp.Format(TEXT("%04X"), pDosHeader->e_lfanew + 22);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pFileHeaders->Characteristics);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;
}

void CPeparserDlg::ShowOptionalHeader()
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)m_pMyPe->GetDosHeaderPointer();
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)m_pMyPe->GetNtHeaderPointer();
	PIMAGE_FILE_HEADER pFileHeaders = (PIMAGE_FILE_HEADER)m_pMyPe->GetFileHeaderPointer();
	PIMAGE_OPTIONAL_HEADER pOptionalHeaders = (PIMAGE_OPTIONAL_HEADER)m_pMyPe->GetOptionHeaderPointer();

	ClearMainListCtrl();
	m_MainListCtrl.InsertColumn(0, TEXT("Member"), LVCFMT_LEFT, 220);
	m_MainListCtrl.InsertColumn(1, TEXT("Offset"), LVCFMT_LEFT, 120);
	m_MainListCtrl.InsertColumn(2, TEXT("Size"), LVCFMT_LEFT, 120);
	m_MainListCtrl.InsertColumn(3, TEXT("Value"), LVCFMT_LEFT, 120);


	int nItem = 0;
	CString csTmp;

	m_MainListCtrl.InsertItem(nItem, TEXT("Magic"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 24);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pOptionalHeaders->Magic);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("MajorLinkerVersion"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 26);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Byte"));
	csTmp.Format(TEXT("%02x"), pOptionalHeaders->MajorLinkerVersion);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("MinorLinkerVersion"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 27);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Byte"));
	csTmp.Format(TEXT("%02x"), pOptionalHeaders->MinorLinkerVersion);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("SizeOfCode"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 28);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->SizeOfCode);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("SizeOfInitializedData"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 32);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->SizeOfInitializedData);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("SizeOfUninitializedData"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 36);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->SizeOfUninitializedData);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("AddressOfEntryPoint"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 40);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->AddressOfEntryPoint);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("BaseOfCode"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 44);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->BaseOfCode);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("BaseOfData"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 48);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->BaseOfData);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("ImageBase"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 52);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->ImageBase);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("SectionAlignment"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 56);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->SectionAlignment);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("FileAlignment"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 60);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->FileAlignment);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("MajorOperatingSystemVersion"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 64);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pOptionalHeaders->MajorOperatingSystemVersion);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("MinorOperatingSystemVersion"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 66);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pOptionalHeaders->MinorOperatingSystemVersion);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("MajorImageVersion"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 68);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pOptionalHeaders->MajorImageVersion);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("MinorImageVersion"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 70);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pOptionalHeaders->MinorImageVersion);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("MajorSubsystemVersion"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 72);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pOptionalHeaders->MajorSubsystemVersion);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("MinorSubsystemVersion"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 74);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pOptionalHeaders->MinorSubsystemVersion);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("Win32VersionValue"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 76);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->Win32VersionValue);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("SizeOfImage"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 80);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->SizeOfImage);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("SizeOfHeaders"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 84);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->SizeOfHeaders);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("CheckSum"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 88);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->CheckSum);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("Subsystem"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 92);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pOptionalHeaders->Subsystem);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("DllCharacteristics"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 94);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pOptionalHeaders->DllCharacteristics);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("SizeOfStackReserve"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 96);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->SizeOfStackReserve);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("SizeOfStackCommit"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 100);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->SizeOfStackCommit);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("SizeOfHeapReserve"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 104);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->SizeOfHeapReserve);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("SizeOfHeapCommit"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 108);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->SizeOfHeapCommit);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("LoaderFlags"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 112);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->LoaderFlags);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

	m_MainListCtrl.InsertItem(nItem, TEXT("NumberOfRvaAndSizes"));
	csTmp.Format(TEXT("%08X"), pDosHeader->e_lfanew + 116);
	m_MainListCtrl.SetItemText(nItem, 1, csTmp);
	m_MainListCtrl.SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pOptionalHeaders->NumberOfRvaAndSizes);
	m_MainListCtrl.SetItemText(nItem, 3, csTmp);
	nItem++;

}

void CPeparserDlg::ShowSectionHeader()
{
	ClearMainListCtrl();
	m_MainListCtrl.InsertColumn(0, TEXT("Name"), LVCFMT_LEFT, 120);
	m_MainListCtrl.InsertColumn(1, TEXT("Virtual Size"), LVCFMT_LEFT, 120);
	m_MainListCtrl.InsertColumn(2, TEXT("Virtual Address"), LVCFMT_LEFT, 120);
	m_MainListCtrl.InsertColumn(3, TEXT("Raw Size"), LVCFMT_LEFT, 120);
	m_MainListCtrl.InsertColumn(4, TEXT("Raw Address"), LVCFMT_LEFT, 120);
	m_MainListCtrl.InsertColumn(5, TEXT("Characteristics"), LVCFMT_LEFT, 120);

	PIMAGE_SECTION_HEADER pSectionHeaders = (PIMAGE_SECTION_HEADER)m_pMyPe->GetSectionHeaderPointer();
	DWORD dwSection = m_pMyPe->GetNumberOfSections();

	int nItem = 0;
	CString csTmp;
	char szTmp[9] = { 0 };
	//wchar_t szTmp2[32] = { 0 };

	for (DWORD i = 0; i < dwSection; i++)
	{
		memcpy(szTmp, pSectionHeaders->Name, 8);
		//swprintf(szTmp2, 32, L"%hs", szTmp);
		m_MainListCtrl.InsertItem(i, szTmp);

		csTmp.Format(TEXT("%08X"), pSectionHeaders->Misc.VirtualSize);
		m_MainListCtrl.SetItemText(i, 1, csTmp);

		csTmp.Format(TEXT("%08X"), pSectionHeaders->VirtualAddress);
		m_MainListCtrl.SetItemText(i, 2, csTmp);

		csTmp.Format(TEXT("%08X"), pSectionHeaders->SizeOfRawData);
		m_MainListCtrl.SetItemText(i, 3, csTmp);

		csTmp.Format(TEXT("%08X"), pSectionHeaders->PointerToRawData);
		m_MainListCtrl.SetItemText(i, 4, csTmp);

		csTmp.Format(TEXT("%08X"), pSectionHeaders->Characteristics);
		m_MainListCtrl.SetItemText(i, 5, csTmp);

		pSectionHeaders = pSectionHeaders + 1;
		::RtlZeroMemory(szTmp, sizeof(szTmp));
		//::RtlZeroMemory(szTmp2, sizeof(szTmp2));
	}
}

void CPeparserDlg::ShowExportDirectory()
{
	ClearDoubleAListCtrl();
	ClearDoubleBListCtrl();

	m_DoubleAListCtrl->InsertColumn(0, TEXT("Member"), LVCFMT_LEFT, 150);
	m_DoubleAListCtrl->InsertColumn(1, TEXT("Offset"), LVCFMT_LEFT, 120);
	m_DoubleAListCtrl->InsertColumn(2, TEXT("Size"), LVCFMT_LEFT, 120);
	m_DoubleAListCtrl->InsertColumn(3, TEXT("Value"), LVCFMT_LEFT, 120);

	m_DoubleBListCtrl->InsertColumn(0, TEXT("Ordinal"), LVCFMT_LEFT, 120);
	m_DoubleBListCtrl->InsertColumn(1, TEXT("Function RVA"), LVCFMT_LEFT, 120);
	m_DoubleBListCtrl->InsertColumn(2, TEXT("Name"), LVCFMT_LEFT, 160);

	PIMAGE_EXPORT_DIRECTORY pExport = (PIMAGE_EXPORT_DIRECTORY)m_pMyPe->GetExportDirectoryPointer();
	if (pExport == NULL)
	{
		return;
	}

	/*
	typedef struct _IMAGE_EXPORT_DIRECTORY {
	  DWORD   Characteristics;            
	  DWORD   TimeDateStamp;          
	  WORD    MajorVersion;               
	  WORD    MinorVersion;           
	  DWORD   Name;                   // dll名称
	  DWORD   Base;                   // 序号查询时会用上，数组的坐标平移
	  DWORD   NumberOfFunctions;      // 有多少个被导出的项    
	  DWORD   NumberOfNames;          // 有多少个被名称导出的项
	  DWORD   AddressOfFunctions;     // 导出地址表，rva
	  DWORD   AddressOfNames;         // 导出名称表，rva
	  DWORD   AddressOfNameOrdinals;  // 导出序号表，rva
	} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;
	*/

	LPVOID lpBase = m_pMyPe->GetDosHeaderPointer();
	int nItem = 0;
	CString csTmp;

	m_DoubleAListCtrl->InsertItem(nItem, TEXT("Characteristics"));
	csTmp.Format(TEXT("%08X"), ((char*)&pExport->Characteristics - (char*)lpBase));
	m_DoubleAListCtrl->SetItemText(nItem, 1, csTmp);
	m_DoubleAListCtrl->SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pExport->Characteristics);
	m_DoubleAListCtrl->SetItemText(nItem, 3, csTmp);
	nItem++;

	m_DoubleAListCtrl->InsertItem(nItem, TEXT("TimeDateStamp"));
	csTmp.Format(TEXT("%08X"), ((char*)&pExport->TimeDateStamp - (char*)lpBase));
	m_DoubleAListCtrl->SetItemText(nItem, 1, csTmp);
	m_DoubleAListCtrl->SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pExport->TimeDateStamp);
	m_DoubleAListCtrl->SetItemText(nItem, 3, csTmp);
	nItem++;

	m_DoubleAListCtrl->InsertItem(nItem, TEXT("MajorVersion"));
	csTmp.Format(TEXT("%08X"), ((char*)&pExport->MajorVersion - (char*)lpBase));
	m_DoubleAListCtrl->SetItemText(nItem, 1, csTmp);
	m_DoubleAListCtrl->SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pExport->MajorVersion);
	m_DoubleAListCtrl->SetItemText(nItem, 3, csTmp);
	nItem++;

	m_DoubleAListCtrl->InsertItem(nItem, TEXT("MinorVersion"));
	csTmp.Format(TEXT("%08X"), ((char*)&pExport->MinorVersion - (char*)lpBase));
	m_DoubleAListCtrl->SetItemText(nItem, 1, csTmp);
	m_DoubleAListCtrl->SetItemText(nItem, 2, TEXT("Word"));
	csTmp.Format(TEXT("%04X"), pExport->MinorVersion);
	m_DoubleAListCtrl->SetItemText(nItem, 3, csTmp);
	nItem++;

	m_DoubleAListCtrl->InsertItem(nItem, TEXT("Name"));
	csTmp.Format(TEXT("%08X"), ((char*)&pExport->Name - (char*)lpBase));
	m_DoubleAListCtrl->SetItemText(nItem, 1, csTmp);
	m_DoubleAListCtrl->SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pExport->Name);
	m_DoubleAListCtrl->SetItemText(nItem, 3, csTmp);
	nItem++;

	m_DoubleAListCtrl->InsertItem(nItem, TEXT("Base"));
	csTmp.Format(TEXT("%08X"), ((char*)&pExport->Base - (char*)lpBase));
	m_DoubleAListCtrl->SetItemText(nItem, 1, csTmp);
	m_DoubleAListCtrl->SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pExport->Base);
	m_DoubleAListCtrl->SetItemText(nItem, 3, csTmp);
	nItem++;

	m_DoubleAListCtrl->InsertItem(nItem, TEXT("NumberOfFunctions"));
	csTmp.Format(TEXT("%08X"), ((char*)&pExport->NumberOfFunctions - (char*)lpBase));
	m_DoubleAListCtrl->SetItemText(nItem, 1, csTmp);
	m_DoubleAListCtrl->SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pExport->NumberOfFunctions);
	m_DoubleAListCtrl->SetItemText(nItem, 3, csTmp);
	nItem++;

	m_DoubleAListCtrl->InsertItem(nItem, TEXT("NumberOfNames"));
	csTmp.Format(TEXT("%08X"), ((char*)&pExport->NumberOfNames - (char*)lpBase));
	m_DoubleAListCtrl->SetItemText(nItem, 1, csTmp);
	m_DoubleAListCtrl->SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pExport->NumberOfNames);
	m_DoubleAListCtrl->SetItemText(nItem, 3, csTmp);
	nItem++;

	m_DoubleAListCtrl->InsertItem(nItem, TEXT("AddressOfFunctions"));
	csTmp.Format(TEXT("%08X"), ((char*)&pExport->AddressOfFunctions - (char*)lpBase));
	m_DoubleAListCtrl->SetItemText(nItem, 1, csTmp);
	m_DoubleAListCtrl->SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pExport->AddressOfFunctions);
	m_DoubleAListCtrl->SetItemText(nItem, 3, csTmp);
	nItem++;

	m_DoubleAListCtrl->InsertItem(nItem, TEXT("AddressOfNames"));
	csTmp.Format(TEXT("%08X"), ((char*)&pExport->AddressOfNames - (char*)lpBase));
	m_DoubleAListCtrl->SetItemText(nItem, 1, csTmp);
	m_DoubleAListCtrl->SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pExport->AddressOfNames);
	m_DoubleAListCtrl->SetItemText(nItem, 3, csTmp);
	nItem++;

	m_DoubleAListCtrl->InsertItem(nItem, TEXT("AddressOfNameOrdinals"));
	csTmp.Format(TEXT("%08X"), ((char*)&pExport->AddressOfNameOrdinals - (char*)lpBase));
	m_DoubleAListCtrl->SetItemText(nItem, 1, csTmp);
	m_DoubleAListCtrl->SetItemText(nItem, 2, TEXT("Dword"));
	csTmp.Format(TEXT("%08X"), pExport->AddressOfNameOrdinals);
	m_DoubleAListCtrl->SetItemText(nItem, 3, csTmp);
	nItem++;

	// 插入导出表的详细信息
	DWORD dwBase = pExport->Base;
	DWORD dwNumberOfFunctions = pExport->NumberOfFunctions;
	DWORD dwNumberOfNames = pExport->NumberOfNames;

	DWORD dwAddressOfFunctions = pExport->AddressOfFunctions;
	DWORD *pAddressOfFunctions = (DWORD*)(m_pMyPe->Rva2Fa(dwAddressOfFunctions) + (char*)m_pMyPe->GetDosHeaderPointer());

	DWORD dwAddressOfNames = pExport->AddressOfNames;
	DWORD* pAddressOfNames = (DWORD*)(m_pMyPe->Rva2Fa(dwAddressOfNames) + (char*)m_pMyPe->GetDosHeaderPointer());

	DWORD dwAddressOfNameOrdinals = pExport->AddressOfNameOrdinals;
	WORD* pAddressOfNameOrdinals = (WORD*)(m_pMyPe->Rva2Fa(dwAddressOfNameOrdinals) + (char*)m_pMyPe->GetDosHeaderPointer());

	for (DWORD i = 0; i < dwNumberOfFunctions; ++i) 
	{
		csTmp.Format(TEXT("%d"), dwBase + i);
		m_DoubleBListCtrl->InsertItem(i, csTmp);
		csTmp.Format(TEXT("%08X"), pAddressOfFunctions[i]);
		m_DoubleBListCtrl->SetItemText(i, 1, csTmp);
		
		LPVOID lpName = m_pMyPe->GetExportName(i);
		if(lpName == NULL)
		{
			csTmp.Format(TEXT("%s"), TEXT("NA"));
		}
		else
		{
			csTmp.Format(TEXT("%s"), (char*)lpName);
		}

		m_DoubleBListCtrl->SetItemText(i, 2, csTmp);
	}

}

void CPeparserDlg::ShowImportDirectory()
{
	ClearDoubleAListCtrl();
	ClearDoubleBListCtrl();

	/*
	typedef struct _IMAGE_IMPORT_DESCRIPTOR {
		union {
			DWORD   Characteristics;            
			DWORD   OriginalFirstThunk;         // 导入名称表(INT)的RVA，GetProAddress
		} DUMMYUNIONNAME;
		DWORD   TimeDateStamp;                  
		DWORD   ForwarderChain;                  
		DWORD   Name;                           // dll名称的地址，LoadLibrary
		DWORD   FirstThunk;                     // 指向导入地址表(IAT)的RVA，pfn填在此处
	} IMAGE_IMPORT_DESCRIPTOR;
	typedef IMAGE_IMPORT_DESCRIPTOR UNALIGNED *PIMAGE_IMPORT_DESCRIPTOR;

	typedef struct _IMAGE_THUNK_DATA32 {
	  union {
		PBYTE  ForwarderString;                 //指向一个转向者字符串的RVA；
		PDWORD Function;                        //导入函数的地址；
		DWORD Ordinal;                          //导入函数的序号；
		PIMAGE_IMPORT_BY_NAME  AddressOfData;   //指向IMAGE_IMPORT_BY_NAME；
	  } u1;
	} IMAGE_THUNK_DATA32;

	`INT表`和`IAT表`都是`IMAGE_THUNK_DATA`结构的

	// IMAGE_THUNK_DATA32 在不同的状态下有不同的解释方式：
	// 在文件状态下解释为 PIMAGE_IMPORT_BY_NAME
	// 进程状态后是函数地址
	// 如果是序号导入的函数，最高位应该为一，取LWORD作为序号

	typedef struct _IMAGE_IMPORT_BY_NAME {
	  WORD Hint;     // 编译器添加的当前电脑中对应函数的序号
	  BYTE Name[1];  // 字符串
	} IMAGE_IMPORT_BY_NAME, *PIMAGE_IMPORT_BY_NAME;

	*/
	m_DoubleAListCtrl->InsertColumn(0, TEXT("Member"), LVCFMT_LEFT, 150);
	m_DoubleAListCtrl->InsertColumn(1, TEXT("Offset"), LVCFMT_LEFT, 120);
	m_DoubleAListCtrl->InsertColumn(2, TEXT("Size"), LVCFMT_LEFT, 120);
	m_DoubleAListCtrl->InsertColumn(3, TEXT("Value"), LVCFMT_LEFT, 120);

	PIMAGE_IMPORT_DESCRIPTOR pImport = (PIMAGE_IMPORT_DESCRIPTOR)m_pMyPe->GetImportDirectoryPointer();
	if (pImport == NULL)
	{
		return;
	}

	// 测试MyGetProcAddress
	CMyPe::MyGetProcAddress((HMODULE)m_pMyPe->GetDosHeaderPointer(), (LPCSTR)1);


}

void CPeparserDlg::ShowResourceDirectory()
{
	ClearDoubleAListCtrl();
	ClearDoubleBListCtrl();

	m_DoubleAListCtrl->InsertColumn(0, TEXT("Member"), LVCFMT_LEFT, 150);
	m_DoubleAListCtrl->InsertColumn(1, TEXT("Offset"), LVCFMT_LEFT, 120);
	m_DoubleAListCtrl->InsertColumn(2, TEXT("Size"), LVCFMT_LEFT, 120);
	m_DoubleAListCtrl->InsertColumn(3, TEXT("Value"), LVCFMT_LEFT, 120);

	PIMAGE_RESOURCE_DIRECTORY pResource = (PIMAGE_RESOURCE_DIRECTORY)m_pMyPe->GetResourceDirectoryPointer();
	if (pResource == NULL)
	{
		return;
	}
}

void CPeparserDlg::ShowRelocDirectory()
{
	ClearDoubleAListCtrl();
	ClearDoubleBListCtrl();

	m_DoubleAListCtrl->InsertColumn(0, TEXT("Member"), LVCFMT_LEFT, 150);
	m_DoubleAListCtrl->InsertColumn(1, TEXT("Offset"), LVCFMT_LEFT, 120);
	m_DoubleAListCtrl->InsertColumn(2, TEXT("Size"), LVCFMT_LEFT, 120);
	m_DoubleAListCtrl->InsertColumn(3, TEXT("Value"), LVCFMT_LEFT, 120);

	PIMAGE_BASE_RELOCATION pReloc = (PIMAGE_BASE_RELOCATION)m_pMyPe->GetRelocDirectoryPointer();
	if (pReloc == NULL)
	{
		return;
	}
}

void CPeparserDlg::ShowTlsDirectory()
{
	ClearDoubleAListCtrl();
	ClearDoubleBListCtrl();

	m_DoubleAListCtrl->InsertColumn(0, TEXT("Member"), LVCFMT_LEFT, 150);
	m_DoubleAListCtrl->InsertColumn(1, TEXT("Offset"), LVCFMT_LEFT, 120);
	m_DoubleAListCtrl->InsertColumn(2, TEXT("Size"), LVCFMT_LEFT, 120);
	m_DoubleAListCtrl->InsertColumn(3, TEXT("Value"), LVCFMT_LEFT, 120);

	PIMAGE_TLS_DIRECTORY pExport = (PIMAGE_TLS_DIRECTORY)m_pMyPe->GetTlsDirectoryPointer();
	if (pExport == NULL)
	{
		return;
	}
}

void CPeparserDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	FinitHeapData();
	CDialogEx::OnClose();
}

void CPeparserDlg::OnSelchangedTreePe(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	if (m_pMyPe == NULL)
	{
		return;
	}

	HTREEITEM hTreeItem = m_TreeCtrl.GetSelectedItem();
	CString csItemText = m_TreeCtrl.GetItemText(hTreeItem);

	if (csItemText == TEXT("Dos Header"))
	{
		ShowDosHeader();
	}

	if (csItemText == TEXT("Nt Header"))
	{
		ShowNtHeader();
	}

	if (csItemText == TEXT("File Header"))
	{
		ShowFileHeader();
	}

	if (csItemText == TEXT("Optional Header"))
	{
		ShowOptionalHeader();
	}

	if (csItemText == TEXT("Section Header"))
	{
		ShowSectionHeader();
	}

	if (csItemText == TEXT("Export Directory"))
	{
		ShowExportDirectory();
	}

	if (csItemText == TEXT("Import Directory"))
	{
		ShowImportDirectory();
	}

	if (csItemText == TEXT("Resource Directory"))
	{
		ShowResourceDirectory();
	}

	if (csItemText == TEXT("Relocation Directory"))
	{
		ShowRelocDirectory();
	}

	if (csItemText == TEXT("TLS Directory"))
	{
		ShowTlsDirectory();
	};
}
