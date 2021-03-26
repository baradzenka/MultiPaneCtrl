/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DemoDlg.h"
#include <new>
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabDialog.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(TabDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// 
void TabDialog::OnBnClickedButton1()
{	::MessageBox(m_hWnd,_T("TabDialog::OnBnClickedButton1"),_T("TabDialog"),MB_OK);
}
// 
void TabDialog::OnCancel()
{	//CDialog::OnCancel();
}
// 
void TabDialog::OnOK()
{	//CDialog::OnOK();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DemoDlg.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(DemoDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_COMBO1, DemoDlg::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUT21, DemoDlg::OnBnClickedBut21)
	ON_BN_CLICKED(IDC_BUT22, DemoDlg::OnBnClickedBut22)
	ON_BN_CLICKED(IDC_BUT23, DemoDlg::OnBnClickedBut23)
	ON_BN_CLICKED(IDC_BUT24, DemoDlg::OnBnClickedBut24)
	ON_BN_CLICKED(IDC_BUT25, DemoDlg::OnBnClickedBut25)
	ON_BN_CLICKED(IDC_BUT26, DemoDlg::OnBnClickedBut26)
	ON_BN_CLICKED(IDC_BUT31, DemoDlg::OnBnClickedBut31)
	ON_BN_CLICKED(IDC_BUT32, DemoDlg::OnBnClickedBut32)
	ON_BN_CLICKED(IDC_BUT33, DemoDlg::OnBnClickedBut33)
	ON_BN_CLICKED(IDC_BUT41, DemoDlg::OnBnClickedBut41)
	ON_BN_CLICKED(IDC_BUT42, DemoDlg::OnBnClickedBut42)
	ON_BN_CLICKED(IDC_BUT43, DemoDlg::OnBnClickedBut43)
	ON_BN_CLICKED(IDC_BUT44, DemoDlg::OnBnClickedBut44)
	ON_BN_CLICKED(IDC_BUT45, DemoDlg::OnBnClickedBut45)
	ON_BN_CLICKED(IDC_BUT46, DemoDlg::OnBnClickedBut46)
	ON_BN_CLICKED(IDC_BUT51, DemoDlg::OnBnClickedBut51)
	ON_BN_CLICKED(IDC_BUT52, DemoDlg::OnBnClickedBut52)
	ON_BN_CLICKED(IDC_BUT53, DemoDlg::OnBnClickedBut53)
	ON_BN_CLICKED(IDC_BUT61, DemoDlg::OnBnClickedBut61)
	ON_BN_CLICKED(IDC_BUT62, DemoDlg::OnBnClickedBut62)
	ON_BN_CLICKED(IDC_BUT81, DemoDlg::OnBnClickedBut81)
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// 
int DemoDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{	if(CDialog::OnCreate(lpCreateStruct)==-1)
		return -1;
	ModifyStyle(0,WS_CLIPCHILDREN);	// to avoid flicks of child controls.
		// 
		// 
		// Creation of MultiPaneCtrl object.
		// 
	if( !m_MPCC.Create(this,WS_CHILD | WS_VISIBLE,CRect(0,0,0,0),3001) )
		return -1;
		// 
		// 
		// Creation of child windows.
		// 
	if( !m_List1.Create(WS_CHILD | LVS_REPORT,CRect(0,0,0,0),this,3002) ||
		!m_List2.Create(WS_CHILD | LVS_REPORT,CRect(0,0,0,0),this,3003) ||
		!m_List3.Create(WS_CHILD | LVS_REPORT,CRect(0,0,0,0),this,3004) )
		return -1;
		// 
	if( !m_Tree1.Create(WS_CHILD | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES,CRect(0,0,0,0),this,3005) ||
		!m_Tree2.Create(WS_CHILD | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES,CRect(0,0,0,0),this,3006) )
		return -1;
		// 
	if( !m_Edit1.Create(WS_CHILD | ES_MULTILINE,CRect(0,0,0,0),this,3007) ||
		!m_Edit2.Create(WS_CHILD | ES_MULTILINE,CRect(0,0,0,0),this,3008) )
		return -1;
		// 
	if( !m_Dlg1.Create(IDD_DIALOG,this) )
		return -1;	// create as modeless dialog box.
	m_Dlg1.SetDlgCtrlID(3009);		// set unique id - important for dialog box.
		// 
		// 
		// Loading state or creation default state.
		// 
	try
	{	MultiPaneCtrl::Tabs tabs;
		tabs.Add(m_Tree1,_T("Tree1"),0);
		tabs.Add(m_List1,_T("List1"),1);
		tabs.Add(m_List2,_T("List2"),2);
		tabs.Add(m_Tree2,_T("Tree2"),3);
		tabs.Add(m_List3,_T("List3"),-1);
		tabs.Add(m_Dlg1,_T("Dlg1"),4);
		tabs.Add(m_Edit1,_T("Edit1"),5);
		tabs.Add(m_Edit2,_T("Edit2"),6);
			// 
		if( !m_MPCC.LoadState(AfxGetApp(),_T("MultiPaneCtrl_DemoDlg"),_T("State"),&tabs,false) )
			SetDefaultLayout(tabs);   // create default state.
	}
	catch(std::bad_alloc &)
	{	return -1;
	}
		// 
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// 
BOOL DemoDlg::OnInitDialog()
{	CDialog::OnInitDialog();
		// 
	SetTabsPosition();
		// 
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
	m_Edit1.SetFont(CFont::FromHandle( static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT)) ));
	m_Edit2.SetWindowText(_T("CEdit 2"));
	m_Edit2.SetFont(CFont::FromHandle( static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT)) ));
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
	m_MPCC.CreateSystemImages(nullptr,IDB_IMAGES_SYSTEM,true,14);
	m_MPCC.CreateImages(nullptr,IDB_IMAGES_TAB_NORMAL,IDB_IMAGES_TAB_DISABLE,true,16);
		// 
		// 
	CFont font;
	font.CreatePointFont(85,_T("Tahoma"));
	m_MPCC.SetFontNormal(&font);
	m_MPCC.SetFontSelect(&font);
		// 
		// 
	m_MPCC.InstallStyle(&m_MPCC.styleBase);
	m_MPCC.SetDockingMarkers( MarkersLayoutB(), 61);
		// 
	m_MPCC.SetAbilityManager(this);
	m_MPCC.SetNotifyManager(this);
		// 
	m_MPCC.SetCursors(IDC_CURSOR_TAB,IDC_CURSOR_SPLITTER_HORZ,IDC_CURSOR_SPLITTER_VERT,IDC_CURSOR_DRAGOUT_ENABLE,IDC_CURSOR_DRAGOUT_DISABLE);
	m_MPCC.EnableTabRemove(true);
	m_MPCC.EnableTabDrag(true);
	m_MPCC.SetPanesMinSize( CSize(20,20) );
		// 
	m_MPCC.Update();
		// 
		// 
		// 
	CComboBox *pCombo = static_cast<CComboBox*>( GetDlgItem(IDC_COMBO1) );
	pCombo->AddString(_T("1. Base style"));
	pCombo->AddString(_T("2. Like client area VS2003"));
	pCombo->AddString(_T("3. Kind of 2"));
	pCombo->AddString(_T("4. Like bars VS2003"));
	pCombo->AddString(_T("5. Kind of 4"));
	pCombo->AddString(_T("6. Like client area VS2008 classic"));
	pCombo->AddString(_T("7. Like client area VS2008 blue"));
	pCombo->AddString(_T("8. Like client area VS2008 silver"));
	pCombo->AddString(_T("9. Like client area VS2008 olive"));
	pCombo->AddString(_T("10. Like bars VS2008 classic"));
	pCombo->AddString(_T("11. Kind of 10"));
	pCombo->AddString(_T("12. Like bars VS2008 blue"));
	pCombo->AddString(_T("13. Kind of 12"));
	pCombo->AddString(_T("14. Like bars VS2008 silver"));
	pCombo->AddString(_T("15. Kind of 14"));
	pCombo->AddString(_T("16. Like bars VS2008 olive"));
	pCombo->AddString(_T("17. Kind of 16"));
	pCombo->AddString(_T("18. Like client area VS2010"));
	pCombo->AddString(_T("19. Like bars VS2010"));
	pCombo->AddString(_T("20. Like client area VS2019 light"));
	pCombo->AddString(_T("21. Kind of 20"));
	pCombo->AddString(_T("22. Like client area VS2019 dark"));
	pCombo->AddString(_T("23. Like client area VS2019 blue"));
	pCombo->AddString(_T("24. Like bars VS2019 light"));
	pCombo->AddString(_T("25. Like bars VS2019 dark"));
	pCombo->AddString(_T("26. Like bars VS2019 blue"));
	pCombo->SetCurSel(0);
		// 
	SetButtonCheck(IDC_BUT21,m_MPCC.GetTabsLayout()==TabCtrl::LayoutTop);
	SetButtonCheck(IDC_BUT22,m_MPCC.GetTabsLayout()==TabCtrl::LayoutBottom);
	SetButtonCheck(IDC_BUT23,m_MPCC.GetTabsBehavior()==TabCtrl::BehaviorScale);
	SetButtonCheck(IDC_BUT24,m_MPCC.GetTabsBehavior()==TabCtrl::BehaviorScroll);
	SetButtonCheck(IDC_BUT25,m_MPCC.GetSplitterDraggingMode()==MultiPaneCtrl::SplitterDraggingStatic);
	SetButtonCheck(IDC_BUT26,m_MPCC.GetSplitterDraggingMode()==MultiPaneCtrl::SplitterDraggingDynamic);
		// 
	SetButtonCheck(IDC_BUT31, m_MPCC.IsBorderVisible() );
	SetButtonCheck(IDC_BUT33,true);
		// 
	SetButtonCheck(IDC_BUT41,true);
	SetButtonCheck(IDC_BUT42, m_MPCC.IsHideSingleTab() );
	SetButtonCheck(IDC_BUT43, m_MPCC.IsTabRemoveEnable() );
	SetButtonCheck(IDC_BUT44, m_MPCC.IsTabDragEnable() );
		// 
	SetButtonCheck(IDC_BUT45, m_MPCC.IsWatchCtrlActivity() );
		// 
	LOGFONT logfont;
	m_MPCC.GetFontSelect()->GetLogFont(&logfont/*out*/);
	SetButtonCheck(IDC_BUT46,(logfont.lfWeight==FW_BOLD));
		// 
	SetButtonCheck(IDC_BUT51, m_MPCC.IsButtonsCloseVisible() );
	SetButtonCheck(IDC_BUT52, m_MPCC.IsButtonsMenuVisible() );
	SetButtonCheck(IDC_BUT53, m_MPCC.IsButtonsScrollVisible() );
		// 
	SetButtonCheck(IDC_BUT71,true);
	SetButtonCheck(IDC_BUT72,true);
	SetButtonCheck(IDC_BUT73,true);
	SetButtonCheck(IDC_BUT74,true);
	SetButtonCheck(IDC_BUT75,true);
		// 
		// 
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DemoDlg::OnDestroy()
{	m_MPCC.SaveState(AfxGetApp(),_T("MultiPaneCtrl_DemoDlg"),_T("State"));
		// 
	CDialog::OnDestroy();
}
/////////////////////////////////////////////////////////////////////////////
// 
void DemoDlg::OnSize(UINT nType, int cx, int cy)
{	CDialog::OnSize(nType, cx, cy);
		// 
	SetTabsPosition();
}
/////////////////////////////////////////////////////////////////////////////
// 
void DemoDlg::SetTabsPosition()
{	CWnd *pBaseWnd = GetDlgItem(IDC_TAB);
		// 
	if(pBaseWnd)
	{	CRect rcBase;
		pBaseWnd->GetWindowRect(&rcBase/*out*/);
		ScreenToClient(&rcBase);
			// 
		CRect rc;
		GetClientRect(&rc/*out*/);
		rc.DeflateRect(rcBase.left,rcBase.top,rcBase.top,rcBase.top);
		m_MPCC.MoveWindow(&rc);
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Style.
void DemoDlg::OnCbnSelchangeCombo1()
{	CComboBox *pCombo = static_cast<CComboBox*>( GetDlgItem(IDC_COMBO1) );
	switch( pCombo->GetCurSel() )
	{	case 0: 
			m_MPCC.InstallStyle(&m_MPCC.styleBase);
			m_MPCC.SetDockingMarkers( MarkersLayoutB(), 61);
			break;
		case 1: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2003_client);
			m_MPCC.DisableDockingMarkers();
			break;
		case 2: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2003_client_custom1);
			m_MPCC.SetDockingMarkers( MarkersLayoutC(), 50);
			break;
		case 3: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2003_bars);
			m_MPCC.DisableDockingMarkers();
			break;
		case 4: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2003_bars_custom1);
			m_MPCC.SetDockingMarkers( MarkersLayoutC(), 50);
			break;
		case 5: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2008_client_classic);
			m_MPCC.SetDockingMarkers( MarkersLayoutA(), 61);
			break;
		case 6: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2008_client_blue);
			m_MPCC.SetDockingMarkers( MarkersLayoutA(), 61);
			break;
		case 7: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2008_client_silver);
			m_MPCC.SetDockingMarkers( MarkersLayoutA(), 61);
			break;
		case 8: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2008_client_olive);
			m_MPCC.SetDockingMarkers( MarkersLayoutA(), 61);
			break;
		case 9: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2008_bars_classic);
			m_MPCC.SetDockingMarkers( MarkersLayoutA(), 61);
			break;
		case 10: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2008_bars_classic_custom1);
			m_MPCC.SetDockingMarkers( MarkersLayoutA(), 61);
			break;
		case 11: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2008_bars_blue);
			m_MPCC.SetDockingMarkers( MarkersLayoutA(), 61);
			break;
		case 12: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2008_bars_blue_custom1);
			m_MPCC.SetDockingMarkers( MarkersLayoutA(), 61);
			break;
		case 13: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2008_bars_silver);
			m_MPCC.SetDockingMarkers( MarkersLayoutA(), 61);
			break;
		case 14: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2008_bars_silver_custom1);
			m_MPCC.SetDockingMarkers( MarkersLayoutA(), 61);
			break;
		case 15: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2008_bars_olive);
			m_MPCC.SetDockingMarkers( MarkersLayoutA(), 61);
			break;
		case 16: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2008_bars_olive_custom1);
			m_MPCC.SetDockingMarkers( MarkersLayoutA(), 61);
			break;
		case 17: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2010_client);
			m_MPCC.SetDockingMarkers( MarkersLayoutC(), 50);
			break;
		case 18: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2010_bars);
			m_MPCC.SetDockingMarkers( MarkersLayoutC(), 50);
			break;
		case 19: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2019_client_light);
			m_MPCC.SetDockingMarkers( MarkersLayoutD(), 50);
			break;
		case 20: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2019_client_light_custom1);
			m_MPCC.SetDockingMarkers( MarkersLayoutD(), 50);
			break;
		case 21: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2019_client_dark);
			m_MPCC.SetDockingMarkers( MarkersLayoutE(), 50);
			break;
		case 22: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2019_client_blue);
			m_MPCC.SetDockingMarkers( MarkersLayoutC(), 50);
			break;
		case 23: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2019_bars_light);
			m_MPCC.SetDockingMarkers( MarkersLayoutD(), 50);
			break;
		case 24: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2019_bars_dark);
			m_MPCC.SetDockingMarkers( MarkersLayoutE(), 50);
			break;
		case 25: 
			m_MPCC.InstallStyle(&m_MPCC.styleVS2019_bars_blue);
			m_MPCC.SetDockingMarkers( MarkersLayoutC(), 50);
			break;
	}
	m_MPCC.Update();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Tabs layout: top.
void DemoDlg::OnBnClickedBut21()
{	m_MPCC.SetTabsLayout(TabCtrl::LayoutTop);
	m_MPCC.Update();
}
//
// Tabs layout: bottom. 
void DemoDlg::OnBnClickedBut22()
{	m_MPCC.SetTabsLayout(TabCtrl::LayoutBottom);
	m_MPCC.Update();
}
/////////////////////////////////////////////////////////////////////////////
// Tabs behavior: scaling.
void DemoDlg::OnBnClickedBut23()
{	m_MPCC.SetTabsBehavior(TabCtrl::BehaviorScale);
	m_MPCC.Update();
}
// 
// Tabs behavior: scrolling.
void DemoDlg::OnBnClickedBut24()
{	m_MPCC.SetTabsBehavior(TabCtrl::BehaviorScroll);
	m_MPCC.Update();
}
/////////////////////////////////////////////////////////////////////////////
// Splitters dragging: static.
void DemoDlg::OnBnClickedBut25()
{	m_MPCC.SetSplitterDraggingMode(MultiPaneCtrl::SplitterDraggingStatic);
	m_MPCC.Update();
}
// 
// Splitters dragging: dynamic.
void DemoDlg::OnBnClickedBut26()
{	m_MPCC.SetSplitterDraggingMode(MultiPaneCtrl::SplitterDraggingDynamic);
	m_MPCC.Update();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Show border.
void DemoDlg::OnBnClickedBut31()
{	m_MPCC.ShowBorder( GetButtonCheck(IDC_BUT31) );
	m_MPCC.Update();
}
/////////////////////////////////////////////////////////////////////////////
// Show controls borders.
void DemoDlg::OnBnClickedBut32()
{	if( GetButtonCheck(IDC_BUT32) )
	{	m_List1.ModifyStyle(0,WS_BORDER);
		m_List2.ModifyStyle(0,WS_BORDER);
		m_List3.ModifyStyle(0,WS_BORDER);
		m_Tree1.ModifyStyle(0,WS_BORDER);
		m_Tree2.ModifyStyle(0,WS_BORDER);
		m_Edit1.ModifyStyle(0,WS_BORDER);
		m_Edit2.ModifyStyle(0,WS_BORDER);
		m_Dlg1.ModifyStyle(0,WS_BORDER);
	}
	else
	{	m_List1.ModifyStyle(WS_BORDER,0);
		m_List2.ModifyStyle(WS_BORDER,0);
		m_List3.ModifyStyle(WS_BORDER,0);
		m_Tree1.ModifyStyle(WS_BORDER,0);
		m_Tree2.ModifyStyle(WS_BORDER,0);
		m_Edit1.ModifyStyle(WS_BORDER,0);
		m_Edit2.ModifyStyle(WS_BORDER,0);
		m_Dlg1.ModifyStyle(WS_BORDER,0);
	}
		// 
	m_List1.SetWindowPos(nullptr,0,0,0,0,SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	m_List2.SetWindowPos(nullptr,0,0,0,0,SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	m_List3.SetWindowPos(nullptr,0,0,0,0,SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	m_Tree1.SetWindowPos(nullptr,0,0,0,0,SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	m_Tree2.SetWindowPos(nullptr,0,0,0,0,SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	m_Edit1.SetWindowPos(nullptr,0,0,0,0,SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	m_Edit2.SetWindowPos(nullptr,0,0,0,0,SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	m_Dlg1.SetWindowPos(nullptr,0,0,0,0,SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}
/////////////////////////////////////////////////////////////////////////////
// Custom cursors.
void DemoDlg::OnBnClickedBut33()
{	if( GetButtonCheck(IDC_BUT33) )
		m_MPCC.SetCursors(IDC_CURSOR_TAB,IDC_CURSOR_SPLITTER_HORZ,IDC_CURSOR_SPLITTER_VERT,IDC_CURSOR_DRAGOUT_ENABLE,IDC_CURSOR_DRAGOUT_DISABLE);
	else
		m_MPCC.SetCursors(static_cast<UINT>(0),0,0,0,0);
}
/////////////////////////////////////////////////////////////////////////////
// Show images.
void DemoDlg::OnBnClickedBut41()
{	if( GetButtonCheck(IDC_BUT41) )
		m_MPCC.CreateImages(nullptr,IDB_IMAGES_TAB_NORMAL,IDB_IMAGES_TAB_DISABLE,true,16);
	else
		m_MPCC.CreateImages(nullptr,0,0,true,0);
		// 
	m_MPCC.Update();
}
/////////////////////////////////////////////////////////////////////////////
// Hide single tab.
void DemoDlg::OnBnClickedBut42()
{	m_MPCC.HideSingleTab( GetButtonCheck(IDC_BUT42) );
	m_MPCC.Update();
}
/////////////////////////////////////////////////////////////////////////////
// Remove tabs.
void DemoDlg::OnBnClickedBut43()
{	const bool remove = GetButtonCheck(IDC_BUT43);
		// 
	m_MPCC.EnableTabRemove(remove);
	m_MPCC.Update();
		// 
	EnableControl(IDC_BUT44,remove);
}
// 
// Drag tabs.
void DemoDlg::OnBnClickedBut44()
{	m_MPCC.EnableTabDrag( GetButtonCheck(IDC_BUT44) );
	m_MPCC.Update();
}
/////////////////////////////////////////////////////////////////////////////
// Watch activity control.
void DemoDlg::OnBnClickedBut45()
{	m_MPCC.WatchCtrlActivity( GetButtonCheck(IDC_BUT45) );
	m_MPCC.Update();
}
// 
// Show selected font.
void DemoDlg::OnBnClickedBut46()
{	if( GetButtonCheck(IDC_BUT46) )
	{	LOGFONT logfont;
		m_MPCC.GetFontNormal()->GetLogFont(&logfont/*out*/);
		logfont.lfWeight = FW_BOLD;
		m_MPCC.SetFontSelect(&logfont);
	}
	else
	{	CFont *font = m_MPCC.GetFontNormal();
		m_MPCC.SetFontSelect(font);
	}
	m_MPCC.Update();
}
/////////////////////////////////////////////////////////////////////////////
// Show close buttons.
void DemoDlg::OnBnClickedBut51()
{	m_MPCC.ShowButtonsClose( GetButtonCheck(IDC_BUT51) );
	m_MPCC.Update();
}
// 
// Show menu buttons.
void DemoDlg::OnBnClickedBut52()
{	m_MPCC.ShowButtonsMenu( GetButtonCheck(IDC_BUT52) );
	m_MPCC.Update();
}
// 
// Show scroll buttons.
void DemoDlg::OnBnClickedBut53()
{	m_MPCC.ShowButtonsScroll( GetButtonCheck(IDC_BUT53) );
	m_MPCC.Update();
}
/////////////////////////////////////////////////////////////////////////////
// Splitters: Active.
void DemoDlg::OnBnClickedBut61()
{	AllSplittersActivate(true);
}
// Splitters: Inactive.
void DemoDlg::OnBnClickedBut62()
{	AllSplittersActivate(false);
}
// 
void DemoDlg::AllSplittersActivate(bool active)
{	for(HPANE h=m_MPCC.GetFirstPane(); h; h=m_MPCC.GetNextPane(h))
	{	HPANE parent = m_MPCC.GetParentPane(h);
		if(parent && h!=m_MPCC.GetLastChildPane(parent))
			m_MPCC.ActivateSplitter(h,active);
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void DemoDlg::OnBnClickedBut81()
{	MultiPaneCtrl::Tabs tabs;
	tabs.Add(m_Tree1,_T("Tree1"),0);
	tabs.Add(m_List1,_T("List1"),1);
	tabs.Add(m_List2,_T("List2"),2);
	tabs.Add(m_Tree2,_T("Tree2"),3);
	tabs.Add(m_List3,_T("List3"),-1);
	tabs.Add(m_Dlg1,_T("Dlg1"),4);
	tabs.Add(m_Edit1,_T("Edit1"),5);
	tabs.Add(m_Edit2,_T("Edit2"),6);
		// 
	m_MPCC.DeleteAllPanes();
	SetDefaultLayout(tabs);
	m_MPCC.Update();
}
/////////////////////////////////////////////////////////////////////////////
// 
void DemoDlg::SetDefaultLayout(MultiPaneCtrl::Tabs const &tabs)
{	HPANE h1 = m_MPCC.ConvertPaneToLine(nullptr,true);
		// 
		HPANE h2 = m_MPCC.ConvertPaneToLine(h1,false);
			// 
			HPANE h3 = m_MPCC.ConvertPaneToLine(h2,true);
				m_MPCC.AddTab(h3,tabs[0]);
				m_MPCC.AddTab(h3,tabs[1]);
				// 
			HPANE h4 = m_MPCC.AddPane(h2);
				m_MPCC.AddTab(h4,tabs[2]);
				// 
		HPANE h5 = m_MPCC.AddPane(h1);
			m_MPCC.AddTab(h5,tabs[3]);
			m_MPCC.AddTab(h5,tabs[4]);
		// 
	HPANE h6 = m_MPCC.AddPane(nullptr);
		// 
		HPANE h7 = m_MPCC.ConvertPaneToLine(h6,false);
			m_MPCC.AddTab(h7,tabs[5]);
			// 
		HPANE h8 = m_MPCC.AddPane(h6);
			m_MPCC.AddTab(h8,tabs[6]);
			m_MPCC.AddTab(h8,tabs[7]);
		// 
	int parts1[] = {60,40};
	m_MPCC.SetLinePortions(nullptr,parts1);
	int parts2[] = {70,30};
	m_MPCC.SetLinePortions(h1,parts2);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void DemoDlg::SetButtonCheck(int id, bool check) const 
{	reinterpret_cast<CButton*>( GetDlgItem(id) )->SetCheck(check ? BST_CHECKED : BST_UNCHECKED);
}
bool DemoDlg::GetButtonCheck(int id) const
{	return reinterpret_cast<CButton*>( GetDlgItem(id) )->GetCheck() == BST_CHECKED;
}
// 
void DemoDlg::EnableControl(int id, bool enable) const
{	CWnd *wnd = GetDlgItem(id);
	enable ? wnd->ModifyStyle(WS_DISABLED,0) : wnd->ModifyStyle(0,WS_DISABLED);
	wnd->Invalidate();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool DemoDlg::CanDrop(MultiPaneCtrl const * /*pCtrl*/, TabCtrl::HTAB /*hTabSrc*/, DockingMarkers::Position dockingSide, HPANE /*hPaneDst*/)
{	switch(dockingSide)
	{	case DockingMarkers::PositionLeft: return GetButtonCheck(IDC_BUT71);
		case DockingMarkers::PositionTop: return GetButtonCheck(IDC_BUT72);
		case DockingMarkers::PositionCenter: return GetButtonCheck(IDC_BUT73);
		case DockingMarkers::PositionBottom: return GetButtonCheck(IDC_BUT74);
		case DockingMarkers::PositionRight: return GetButtonCheck(IDC_BUT75);
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void DemoDlg::OnTabButtonCloseClicked(MultiPaneCtrl *paneCtrl, TabCtrl *tabCtrl, CRect const * /*rect*/, CPoint /*ptScr*/)
{	TabCtrl::HTAB tab = tabCtrl->GetSelectedTab();   // get handle of current active tab (whose child window is visible).
	if(tab)
	{	tabCtrl->DeleteTab(tab);
		if( tabCtrl->GetNumberTabs()>0 )
			tabCtrl->Update();
		else
		{	HPANE pane = paneCtrl->GetPaneWithTabCtrl(tabCtrl);
			if( paneCtrl->GetParentPane(pane) )   // you cannot delete the root pane.
				paneCtrl->DeleteOptimizDown(pane);
			paneCtrl->Update();
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void DemoDlg::OnTabButtonMenuClicked(MultiPaneCtrl * /*paneCtrl*/, TabCtrl *tabCtrl, CRect const *rect, CPoint /*ptScr*/)
{	CMenu menu;
	if( menu.CreatePopupMenu() )
	{	const int number = tabCtrl->GetNumberTabs();
		for(int i=0; i<number; ++i)
		{	TabCtrl::HTAB tab = tabCtrl->GetTabHandleByIndex(i);
			const CString text = tabCtrl->GetTabText(tab);
				// 
			MENUITEMINFO info;
			info.cbSize = sizeof(info);
			info.fMask = MIIM_ID | MIIM_STATE | MIIM_TYPE;
			info.wID = i+1;
			info.fState = (!tabCtrl->IsTabDisabled(tab) ? MFS_ENABLED : MFS_DISABLED);
			info.fType = MFT_STRING;
			if(tab==tabCtrl->GetSelectedTab())
			{	info.fState |= MFS_CHECKED;
				info.fType |= MFT_RADIOCHECK;
				info.hbmpChecked = nullptr;
			}
			info.dwTypeData = const_cast<TCHAR *>( text.GetString() );
			info.cch = text.GetLength();
			::InsertMenuItem(menu,i,TRUE,&info);
		}
			// 
		CRect rc(rect);
		tabCtrl->ClientToScreen(&rc);
		const int id = static_cast<int>( ::TrackPopupMenu(menu,TPM_RIGHTALIGN | TPM_RETURNCMD,rc.right,rc.bottom,0,m_hWnd,nullptr) );
		if(id)
		{	TabCtrl::HTAB selTab = tabCtrl->GetTabHandleByIndex(id-1);
			tabCtrl->SelectTab(selTab);
			tabCtrl->EnsureTabVisible(selTab);
			tabCtrl->Update();
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
















