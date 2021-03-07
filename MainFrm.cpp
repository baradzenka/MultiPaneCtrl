// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MainFrm.h"
#include <new>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
		// 
	ON_UPDATE_COMMAND_UI(ID_TESTDLG, OnUpdateTestdlg)
	ON_COMMAND(ID_TESTDLG, OnTestdlg)
		// 
	ON_UPDATE_COMMAND_UI(ID_LOADSTATE_STATE1, CMainFrame::OnUpdateLoadstateState1)
	ON_COMMAND(ID_LOADSTATE_STATE1, CMainFrame::OnLoadstateState1)
	ON_UPDATE_COMMAND_UI(ID_LOADSTATE_STATE2, CMainFrame::OnUpdateLoadstateState2)
	ON_COMMAND(ID_LOADSTATE_STATE2, CMainFrame::OnLoadstateState2)
	ON_UPDATE_COMMAND_UI(ID_LOADSTATE_STATE3, CMainFrame::OnUpdateLoadstateState3)
	ON_COMMAND(ID_LOADSTATE_STATE3, CMainFrame::OnLoadstateState3)
	ON_UPDATE_COMMAND_UI(ID_SAVESTATE_STATE1, CMainFrame::OnUpdateSavestateState1)
	ON_COMMAND(ID_SAVESTATE_STATE1, CMainFrame::OnSavestateState1)
	ON_UPDATE_COMMAND_UI(ID_SAVESTATE_STATE2, CMainFrame::OnUpdateSavestateState2)
	ON_COMMAND(ID_SAVESTATE_STATE2, CMainFrame::OnSavestateState2)
	ON_UPDATE_COMMAND_UI(ID_SAVESTATE_STATE3, CMainFrame::OnUpdateSavestateState3)
	ON_COMMAND(ID_SAVESTATE_STATE3, CMainFrame::OnSavestateState3)
	ON_UPDATE_COMMAND_UI(ID_SHOWCLIENTEDGEBORDER, CMainFrame::OnUpdateShowclientedgeborder)
	ON_COMMAND(ID_SHOWCLIENTEDGEBORDER, CMainFrame::OnShowclientedgeborder)
END_MESSAGE_MAP()

static UINT indicators[] =
{	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// 
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
		// 
		// 
		// Creation of child windows.
		// 
	if( !m_Tree1.Create(WS_CHILD | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES,CRect(0,0,0,0),this,300) ||
		!m_Tree2.Create(WS_CHILD | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES,CRect(0,0,0,0),this,301) ||
		!m_Edit1.Create(WS_CHILD | ES_MULTILINE,CRect(0,0,0,0),this,302) ||
		!m_Edit2.Create(WS_CHILD | ES_MULTILINE,CRect(0,0,0,0),this,303) ||
		!m_List1.Create(WS_CHILD | LVS_REPORT,CRect(0,0,0,0),this,304) ||
		!m_List2.Create(WS_CHILD | LVS_REPORT,CRect(0,0,0,0),this,305) ||
		!m_List3.Create(WS_CHILD | LVS_REPORT,CRect(0,0,0,0),this,306) )
		return -1;
		// 
	HTREEITEM hParent = m_Tree1.InsertItem(_T("CTreeCtrl 1"));
	m_Tree1.InsertItem(_T("Item 1"),hParent);
	m_Tree1.InsertItem(_T("Item 2"),hParent);
	m_Tree1.Expand(hParent,TVE_EXPAND);
		// 
	hParent = m_Tree2.InsertItem(_T("CTreeCtrl 2"));
	m_Tree2.InsertItem(_T("Item 1"),hParent);
	m_Tree2.InsertItem(_T("Item 2"),hParent);
	m_Tree2.Expand(hParent,TVE_EXPAND);
		// 
	m_Edit1.SetWindowText(_T("CEdit 1"));
	m_Edit1.SetFont( CFont::FromHandle(static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT))) );
	m_Edit2.SetWindowText(_T("CEdit 2"));
	m_Edit2.SetFont( CFont::FromHandle(static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT))) );
		// 
	m_List1.InsertColumn(0,_T("CListCtrl 1"),LVCFMT_LEFT,100);
	m_List1.InsertItem(0,_T("Item 1"));
	m_List1.InsertItem(1,_T("Item 2"));
	m_List2.InsertColumn(0,_T("CListCtrl 2"),LVCFMT_LEFT,100);
	m_List2.InsertItem(0,_T("Item 1"));
	m_List2.InsertItem(1,_T("Item 2"));
	m_List3.InsertColumn(0,_T("CListCtrl 3"),LVCFMT_LEFT,100);
	m_List3.InsertItem(0,_T("Item 1"));
	m_List3.InsertItem(1,_T("Item 2"));
		// 
		// 
		// 
		// Creation of MultiPaneCtrl object.
		// 
	if( !m_MultiPaneCtrl.Create(this,WS_CHILD | WS_VISIBLE,CRect(0,0,0,0),AFX_IDW_PANE_FIRST) )
		return -1;
	m_MultiPaneCtrl.SetNotifyManager(this);
		// 
	m_MultiPaneCtrl.CreateSystemImages(nullptr,IDB_IMAGES_SYSTEM,true,14);
	m_MultiPaneCtrl.CreateImages(nullptr,IDB_IMAGES_TAB_NORMAL,IDB_IMAGES_TAB_DISABLE,true,16);
		// 
	m_MultiPaneCtrl.SetCursors(IDC_CURSOR_TAB,IDC_CURSOR_SPLITTER_HORZ,IDC_CURSOR_SPLITTER_VERT,IDC_CURSOR_DRAGOUT_ENABLE,IDC_CURSOR_DRAGOUT_DISABLE);
		// 
	CFont font;
	font.CreatePointFont(85,_T("Tahoma"));
	m_MultiPaneCtrl.SetFontNormal(&font);
	m_MultiPaneCtrl.SetFontSelect(&font);
		// 
	m_MultiPaneCtrl.SetDockingMarkers( MarkersLayoutC(), 50);
	m_MultiPaneCtrl.SetTabsBehavior(TabCtrl::BehaviorScroll);
	m_MultiPaneCtrl.EnableTabRemove(true);
	m_MultiPaneCtrl.EnableTabDrag(true);
		// 
		// Loading state or creation default state.
		// 
	try
	{	MultiPaneCtrl::Tabs tabs;
		tabs.Add(m_Tree1,_T("Tree1"),-1);
		tabs.Add(m_List1,_T("List1"),0);
		tabs.Add(m_Edit1,_T("Edit1"),1);
		tabs.Add(m_List2,_T("List2"),2);
		tabs.Add(m_Tree2,_T("Tree2"),3);
		tabs.Add(m_List3,_T("List3"),4);
		tabs.Add(m_Edit2,_T("Edit2"),5);
			// 
		if( !m_MultiPaneCtrl.LoadState(AfxGetApp(),_T("MultiPaneCtrl"),_T("State"),&tabs,false) )
		{		// create default state.
			HPANE h1 = m_MultiPaneCtrl.ConvertPaneToLine(nullptr,false);
				m_MultiPaneCtrl.AddTab(h1,tabs[0]);
				m_MultiPaneCtrl.AddTab(h1,tabs[1]);
				// 
			HPANE h2 = m_MultiPaneCtrl.AddPane(nullptr);
					// 
				HPANE h3 = m_MultiPaneCtrl.ConvertPaneToLine(h2,true);
					m_MultiPaneCtrl.AddTab(h3,tabs[2]);
					// 
				HPANE h4 = m_MultiPaneCtrl.AddPane(h2);
					HPANE h5 = m_MultiPaneCtrl.ConvertPaneToLine(h4,false);
						m_MultiPaneCtrl.AddTab(h5,tabs[3]);
						// 
					HPANE h6 = m_MultiPaneCtrl.AddPane(h4);
						m_MultiPaneCtrl.AddTab(h6,tabs[4]);
						// 
					HPANE h7 = m_MultiPaneCtrl.AddPane(h4);
						m_MultiPaneCtrl.AddTab(h7,tabs[5]);
					// 
				HPANE h8 = m_MultiPaneCtrl.AddPane(h2);
					m_MultiPaneCtrl.AddTab(h8,tabs[6]);
				// 
			m_MultiPaneCtrl.SetLinesEqualPanesSize();
		}
	}
	catch(std::bad_alloc &)
	{	return -1;
	}
		// 
	m_MultiPaneCtrl.Update();
		// 
		// 
		// 
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
		// 
		// 
		// 
	OnTestdlg();
	m_TestDlg.RedrawWindow(0,0,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		// 
		// 
		// 
	return 0;
}
// 
void CMainFrame::OnDestroy()
{	m_MultiPaneCtrl.SaveState(AfxGetApp(),_T("MultiPaneCtrl"),_T("State"));
		// 
	CFrameWnd::OnDestroy();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void CMainFrame::OnUpdateTestdlg(CCmdUI *pCmdUI)
{	pCmdUI->Enable(!::IsWindow(m_TestDlg.m_hWnd) || !::IsWindowVisible(m_TestDlg.m_hWnd));
}
// 
void CMainFrame::OnTestdlg()
{	if(!m_TestDlg.m_hWnd)
	{	if( !m_TestDlg.Create(IDD_TESTDLG,this) )
			return;
		m_TestDlg.CenterWindow(this);
	}
	m_TestDlg.ShowWindow(SW_SHOWNORMAL);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void CMainFrame::OnUpdateLoadstateState1(CCmdUI * /*pCmdUI*/)
{
}
void CMainFrame::OnLoadstateState1()
{	MultiPaneCtrl::Tabs tabs;
	if( m_MultiPaneCtrl.GetAllTabs(&tabs/*out*/) )   // load information about tabs.
		m_MultiPaneCtrl.LoadStateAndUpdate(AfxGetApp(),_T("MultiPaneCtrl"),_T("State 1"),&tabs,false);
}
// 
void CMainFrame::OnUpdateLoadstateState2(CCmdUI * /*pCmdUI*/)
{
}
void CMainFrame::OnLoadstateState2()
{	MultiPaneCtrl::Tabs tabs;
	if( m_MultiPaneCtrl.GetAllTabs(&tabs/*out*/) )   // load information about tabs.
		m_MultiPaneCtrl.LoadStateAndUpdate(AfxGetApp(),_T("MultiPaneCtrl"),_T("State 2"),&tabs,false);
}
// 
void CMainFrame::OnUpdateLoadstateState3(CCmdUI * /*pCmdUI*/)
{
}
void CMainFrame::OnLoadstateState3()
{	MultiPaneCtrl::Tabs tabs;
	if( m_MultiPaneCtrl.GetAllTabs(&tabs/*out*/) )   // load information about tabs.
		m_MultiPaneCtrl.LoadStateAndUpdate(AfxGetApp(),_T("MultiPaneCtrl"),_T("State 3"),&tabs,false);
}
/////////////////////////////////////////////////////////////////////////////
// 
void CMainFrame::OnUpdateSavestateState1(CCmdUI * /*pCmdUI*/)
{
}
void CMainFrame::OnSavestateState1()
{	m_MultiPaneCtrl.SaveState(AfxGetApp(),_T("MultiPaneCtrl"),_T("State 1"));
}
// 
void CMainFrame::OnUpdateSavestateState2(CCmdUI * /*pCmdUI*/)
{
}
void CMainFrame::OnSavestateState2()
{	m_MultiPaneCtrl.SaveState(AfxGetApp(),_T("MultiPaneCtrl"),_T("State 2"));
}
// 
void CMainFrame::OnUpdateSavestateState3(CCmdUI * /*pCmdUI*/)
{
}
void CMainFrame::OnSavestateState3()
{	m_MultiPaneCtrl.SaveState(AfxGetApp(),_T("MultiPaneCtrl"),_T("State 3"));
}
/////////////////////////////////////////////////////////////////////////////
// 
void CMainFrame::OnUpdateShowclientedgeborder(CCmdUI *pCmdUI)
{	const long style = ::GetWindowLong(m_MultiPaneCtrl.m_hWnd,GWL_EXSTYLE);
	pCmdUI->SetCheck( (style & WS_EX_CLIENTEDGE)!=0 );
}
void CMainFrame::OnShowclientedgeborder()
{	const long style = ::GetWindowLong(m_MultiPaneCtrl.m_hWnd,GWL_EXSTYLE);
		// 
	if(style & WS_EX_CLIENTEDGE)
		::SetWindowLong(m_MultiPaneCtrl.m_hWnd,GWL_EXSTYLE,(style & ~WS_EX_CLIENTEDGE));
	else
		::SetWindowLong(m_MultiPaneCtrl.m_hWnd,GWL_EXSTYLE,(style | WS_EX_CLIENTEDGE));
	m_MultiPaneCtrl.SetWindowPos(nullptr, 0,0,0,0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);	// border frame update.
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void CMainFrame::OnTabButtonCloseClicked(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, CRect const * /*pRect*/, CPoint /*ptScr*/)
{	::MessageBox(m_hWnd,_T("CMainFrame::OnTabButtonCloseClicked"),_T("CMainFrame"),MB_OK);
}
// 
void CMainFrame::OnTabButtonMenuClicked(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, CRect const * /*pRect*/, CPoint /*ptScr*/)
{	::MessageBox(m_hWnd,_T("CMainFrame::OnTabButtonMenuClicked"),_T("CMainFrame"),MB_OK);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////












BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	m_MultiPaneCtrl.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_MultiPaneCtrl.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG











