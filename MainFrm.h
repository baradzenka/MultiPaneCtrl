#pragma once

#include "DemoDlg.h"


class CMainFrame : public CFrameWnd,
	MultiPaneCtrl::Notify
{
	DECLARE_DYNAMIC(CMainFrame)

#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
		// 
	CTreeCtrl m_Tree1, m_Tree2;
	CEdit m_Edit1, m_Edit2;
	CListCtrl m_List1, m_List2, m_List3;
		// 
	MultiPaneCtrlEx<MultiPaneCtrlStyle_VS2003_client_custom1> m_MultiPaneCtrl;
	typedef MultiPaneCtrl::HPANE HPANE;
	DemoDlg m_TestDlg;

private:   // MultiPaneCtrlNotify.
	void OnTabButtonCloseClicked(MultiPaneCtrl *pPaneCtrl, TabCtrl *pTabCtrl, CRect const *pRect, CPoint ptScr) override;
	void OnTabButtonMenuClicked(MultiPaneCtrl *pPaneCtrl, TabCtrl *pTabCtrl, CRect const *pRect, CPoint ptScr) override;

protected:
	DECLARE_MESSAGE_MAP()
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) override;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnUpdateTestdlg(CCmdUI *pCmdUI);
	afx_msg void OnTestdlg();
	afx_msg void OnUpdateLoadstateState1(CCmdUI *pCmdUI);
	afx_msg void OnLoadstateState1();
	afx_msg void OnUpdateLoadstateState2(CCmdUI *pCmdUI);
	afx_msg void OnLoadstateState2();
	afx_msg void OnUpdateLoadstateState3(CCmdUI *pCmdUI);
	afx_msg void OnLoadstateState3();
	afx_msg void OnUpdateSavestateState1(CCmdUI *pCmdUI);
	afx_msg void OnSavestateState1();
	afx_msg void OnUpdateSavestateState2(CCmdUI *pCmdUI);
	afx_msg void OnSavestateState2();
	afx_msg void OnUpdateSavestateState3(CCmdUI *pCmdUI);
	afx_msg void OnSavestateState3();
	afx_msg void OnUpdateShowclientedgeborder(CCmdUI *pCmdUI);
	afx_msg void OnShowclientedgeborder();
};



