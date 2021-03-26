//==========================================================
// Author: Baradzenka Aleh (baradzenka@gmail.com)
//==========================================================
// 
#pragma once
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include "TabCtrl.h"
#include "DockingMarkers.h"
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
class MultiPaneCtrl : public CWnd
{	DECLARE_DYNCREATE(MultiPaneCtrl)

///////////////////////////////////////
// PUBLIC
///////////////////////////////////////
public:
	typedef struct HPANE__ {} const *HPANE;

public:
	struct Draw
	{	virtual void DrawBegin(MultiPaneCtrl const * /*paneCtrl*/, CDC * /*dc*/) {}
		virtual void DrawSplitter(MultiPaneCtrl const * /*paneCtrl*/, CDC * /*dc*/, bool /*horz*/, CRect const * /*rect*/) {}
		virtual void DrawSplitterDragRect(MultiPaneCtrl const * /*paneCtrl*/, CDC * /*dc*/, bool /*horz*/, CRect const * /*rectNew*/, CRect const * /*rectOld*/) {}
		virtual void DrawBorder(MultiPaneCtrl const * /*paneCtrl*/, CDC * /*dc*/, CRect const * /*rect*/) {}
		virtual void DrawEnd(MultiPaneCtrl const * /*paneCtrl*/, CDC * /*dc*/) {}
	};
	interface IRecalc
	{	virtual int GetBorderWidth(MultiPaneCtrl const *paneCtrl, IRecalc *base) = 0;
		virtual CSize GetSplitterSize(MultiPaneCtrl const *paneCtrl, IRecalc *base) = 0;   // width (CSize::cx) for vertical and height (CSize::cy) for horizontal splitter.
	};
	struct Ability
	{		// function is called during Drag and Drop operation.
		virtual bool CanDrop(MultiPaneCtrl const * /*paneCtrl*/, TabCtrl::HTAB /*hTabSrc*/, DockingMarkers::Position /*dockingSide*/, HPANE /*paneDst*/) { return true; }   // permission to insert the panel using a marker under the cursor.
			// functions are called during the recalculation of the control (WM_SIZE or Update()).
		virtual bool CanShowButtonClose(MultiPaneCtrl const * /*paneCtrl*/, TabCtrl const * /*tabCtrl*/) { return true; }
		virtual bool CanShowButtonMenu(MultiPaneCtrl const * /*paneCtrl*/, TabCtrl const * /*tabCtrl*/) { return true; }
		virtual bool CanShowButtonScroll(MultiPaneCtrl const * /*paneCtrl*/, TabCtrl const * /*tabCtrl*/) { return true; }
	};
	struct Notify
	{	virtual void OnTabPreCreate(MultiPaneCtrl * /*paneCtrl*/, TabCtrl const * /*tabCtrl*/, HWND /*wnd*/, TCHAR const * /*text*/, int /*image*/) {}
		virtual void OnTabPostCreate(MultiPaneCtrl * /*paneCtrl*/, TabCtrl * /*tabCtrl*/, TabCtrl::HTAB /*tab*/) {}
		virtual void OnTabPreDestroy(MultiPaneCtrl * /*paneCtrl*/, TabCtrl const * /*tabCtrl*/, TabCtrl::HTAB /*tab*/) {}
			// 
		virtual void OnTabButtonCloseClicked(MultiPaneCtrl * /*paneCtrl*/, TabCtrl * /*tabCtrl*/, CRect const * /*rect*/, CPoint /*ptScr*/) {}   // ptScr - in screen space.
		virtual void OnTabButtonMenuClicked(MultiPaneCtrl * /*paneCtrl*/, TabCtrl * /*tabCtrl*/, CRect const * /*rect*/, CPoint /*ptScr*/) {}   // ptScr - in screen space.
		virtual void OnTabSelected(MultiPaneCtrl * /*paneCtrl*/, TabCtrl * /*tabCtrl*/, TabCtrl::HTAB /*tab*/) {}
		virtual void OnTabLButtonDown(MultiPaneCtrl * /*paneCtrl*/, TabCtrl const * /*tabCtrl*/, TabCtrl::HTAB /*tab*/, CPoint /*ptScr*/) {}   // ptScr - in screen space.
		virtual void OnTabLButtonDblClk(MultiPaneCtrl * /*paneCtrl*/, TabCtrl * /*tabCtrl*/, TabCtrl::HTAB /*tab*/, CPoint /*ptScr*/) {}   // ptScr - in screen space.
		virtual void OnTabRButtonDown(MultiPaneCtrl * /*paneCtrl*/, TabCtrl * /*tabCtrl*/, TabCtrl::HTAB /*tab*/, CPoint /*ptScr*/) {}   // ptScr - in screen space, tab can be null.
		virtual void OnTabRButtonUp(MultiPaneCtrl * /*paneCtrl*/, TabCtrl * /*tabCtrl*/, TabCtrl::HTAB /*tab*/, CPoint /*ptScr*/) {}   // ptScr - in screen space, tab can be null.
			// 
		virtual void OnTabStartDrag(MultiPaneCtrl * /*paneCtrl*/, TabCtrl const * /*tabCtrl*/, TabCtrl::HTAB /*tab*/, CPoint /*ptScr*/) {}   // ptScr - in screen space.
		virtual void OnTabDrag(MultiPaneCtrl * /*paneCtrl*/, TabCtrl * /*tabCtrl*/, TabCtrl::HTAB /*tab*/, CPoint /*ptScr*/, bool /*outside*/) {}   // ptScr - in screen space, outside==true - dragging out of tabs area.
		virtual void OnTabFinishDrag(MultiPaneCtrl * /*paneCtrl*/, TabCtrl * /*tabCtrl*/, TabCtrl::HTAB /*tab*/, bool /*cancel*/) {}   // cancel==false - dragging was finished using left button up.
			// 
		virtual void OnPanePostCreate(MultiPaneCtrl * /*paneCtrl*/, HPANE /*pane*/) {}   // new pane has been created.
		virtual void OnPanePreDestroy(MultiPaneCtrl * /*paneCtrl*/, HPANE /*pane*/) {}   // before pane delete.
	};
		// 
	interface IStyle
	{	virtual ITabCtrlStyle *GetTabCtrlStyle() = 0;
		virtual DockingMarkers::Draw *GetDockMarkersDrawManager() = 0;
			// 
		virtual Draw *GetDrawManager() = 0;
		virtual IRecalc *GetRecalcManager() = 0;
	};

public:
	MultiPaneCtrl();
	~MultiPaneCtrl();

public:
	bool Create(CWnd *parent, DWORD style, RECT const &rect, UINT id);
		// 
	enum Space   // algorithm of insertion or deletion pane.
	{	SpaceBetween,   // takes/returns space from/to previous and next panes.
		SpacePrevious,   // takes/returns space from/to previous pane.
		SpaceNext   // takes/returns space from/to next pane.
	};
	HPANE ConvertPaneToLine(HPANE pane, bool horz);   // convert pane (empty or with tabs) to line, return new created child pane for the passed pane.
	HPANE AddPane(HPANE parent);   // add empty pane to the end of line.
	HPANE InsertPane(HPANE before, Space space);   // insert empty pane to the line.
	void RemovePane(HPANE before, HPANE src);   // change position of 'src' pane in the line.
	void DeletePane(HPANE pane, Space space);   // delete the pane with the redistribution of its space.
	void DeleteAllPanes();   // remove all panes except the root pane.
		// 
	HPANE InsertIntermediatePane(HPANE parent, bool horz);   // insert intermediate pane in hierarchical branch as child pane for the 'parent'.
	void DeleteIntermediatePane(HPANE pane);
		// 
	void Update();   // recalculate and update control (includes call UpdateAllTabs()).
	void Update(HPANE pane);   // without recalculate of pane, only update all its child controls (and descendants).
	void UpdateAllTabs();   // update all child TabCtrl in the control.
		// 
	void InstallStyle(IStyle *style);
		// 
	void SetDrawManager(Draw *p/*or null*/);
	Draw *GetDrawManager() const;
	void SetRecalcManager(IRecalc *p/*or null*/);   // null for default manager.
	IRecalc *GetRecalcManager() const;
	void SetAbilityManager(Ability *p/*or null*/);   // null for default manager.
	Ability *GetAbilityManager() const;
	void SetNotifyManager(Notify *p/*or null*/);
	Notify *GetNotifyManager() const;
		// 
	bool SetDockingMarkers(DockingMarkers::Layout const &layout, int insertMarkerTransp/*1...100%*/);   // insertMarkerTransp - transparency of the window-marker that indicates the position of the pane to insert.
	void DisableDockingMarkers();
	bool IsDockingMarkersEnable() const;
		// 
	bool CreateSystemImages(HMODULE moduleRes/*or null*/, UINT resID/*or 0*/, bool pngImage, int imageWidth, COLORREF clrTransp=CLR_NONE);   // system images must have the same order as TabCtrl::SysImage enum.
	Gdiplus::Bitmap *GetSystemImages() const;
	CSize GetSystemImageSize() const;
	COLORREF GetSystemImagesTranspColor() const;
		// 
	bool CreateImages(HMODULE moduleRes/*or null*/, UINT resNormalID/*or 0*/, UINT resDisableID/*or 0*/, bool pngImage, int imageWidth, COLORREF clrTransp=CLR_NONE);
	void GetImages(Gdiplus::Bitmap **normal/*out,or null*/, Gdiplus::Bitmap **disable/*out,or null*/) const;
	void GetImageSize(CSize *szNormal/*out,or null*/, CSize *szDisable/*out,or null*/) const;
	COLORREF GetImagesTranspColor() const;
		// 
	bool SetCursors(UINT tab/*or 0*/, UINT splitterHorz/*or 0*/, UINT splitterVert/*or 0*/, UINT dragEnable/*or 0*/, UINT dragDisable/*or 0*/);   // 0 if you do not use cursor.
	bool SetCursors(HMODULE module, UINT tab/*or 0*/, UINT splitterHorz/*or 0*/, UINT splitterVert/*or 0*/, UINT dragEnable/*or 0*/, UINT dragDisable/*or 0*/);   // 'module' - module containing the resource of cursor, use 0 for any id if you do not use this cursor.
	bool SetCursors(HCURSOR tab/*or null*/, HCURSOR splitterHorz/*or null*/, HCURSOR splitterVert/*or null*/, HCURSOR dragEnable/*or null*/, HCURSOR dragDisable/*or null*/);   // null if you do not use cursor.
	void SetCursorsRef(HCURSOR *tab/*or null*/, HCURSOR *splitterHorz/*or null*/, HCURSOR *splitterVert/*or null*/, HCURSOR *dragEnable/*or null*/, HCURSOR *dragDisable/*or null*/);   // set references to other cursors.
	void GetCursors(HCURSOR *tab/*out,or null*/, HCURSOR *splitterHorz/*out,or null*/, HCURSOR *splitterVert/*out,or null*/, HCURSOR *dragEnable/*out,or null*/, HCURSOR *dragDisable/*out,or null*/) const;   // any pointer can be null.
		// 
	bool SetFontNormal(CFont *font);
	void SetFontNormalRef(CFont *font);   // set reference to another font.
	bool SetFontNormal(LOGFONT const *lf);
	CFont *GetFontNormal();
		// 
	bool SetFontSelect(CFont *font);
	void SetFontSelectRef(CFont *font);   // set reference to another select font.
	bool SetFontSelect(LOGFONT const *lf);
	CFont *GetFontSelect();
		// 
	void SetPaneData(HPANE pane, __int64 data);
	__int64 GetPaneData(HPANE pane) const;
		// 
	HPANE GetRootPane() const;   // for all functions can be used null instead GetRootPane().
	HPANE GetParentPane(HPANE pane) const;   // return null if parent of 'pane' is root pane.
	HPANE GetPaneHandleByIndex(HPANE parent, int idx) const;   // get pane with the index 'idx' in the line 'parent'.
	int GetPaneIndexByHandle(HPANE pane) const;   // get index of 'pane' in the parent line.
	int GetNumberPanes(HPANE parent) const;   // get number of child panes in the line 'parent'.
	bool IsPaneExist(HPANE pane) const;   // return true - pane with this handle exists in the control.
		// 
	HPANE GetFirstChildPane(HPANE parent) const;   // get first (top for vertical and left for horizontal line) pane in the line 'parent'.
	HPANE GetPrevChildPane(HPANE pane) const;   // get previous pane in the parent line or null if 'pane' is the first pane.
	HPANE GetNextChildPane(HPANE pane) const;   // get next pane in the parent line or null if 'pane' is the last pane.
	HPANE GetLastChildPane(HPANE parent) const;   // get last (bottom for vertical and right for horizontal line) pane in the line 'parent'.
		// 
	HPANE GetFirstPane() const;   // get top pane in the tree of panes (always return the root pane (GetRootPane())).
	HPANE GetPrevPane(HPANE pane) const;   // get previous pane in the tree of panes (recursively).
	HPANE GetNextPane(HPANE pane) const;   // get next pane in the tree of panes (recursively).
	HPANE GetLastPane() const;   // get bottom pane in the tree of panes.
		// 
	HPANE HitTest(CPoint ptScr) const;   // get pane in the given point or null, 'ptScr' - in screen space.
	bool IsLine(HPANE pane) const;   // return true - pane has child panes, return false - pane is empty or has tabs.
	bool IsLineHorizontal(HPANE pane) const;   // get orientation of line (must be IsLine(pane)==true).
	void SetLineOrientation(HPANE pane, bool horz);   // set orientation of line. 
		// 
	void ShowBorder(bool show);   // border is visible if IsBorderVisible()==true and MultiPaneCtrl::IRecalc::GetBorderWidth(...) returns >0.
	bool IsBorderVisible() const;
		// 
	void EnableTabDrag(bool enable);   // enable/disable Drag and Drop operation, drag of tabs works only if IsTabDragEnable()==true and EnableTabRemove()==true.
	bool IsTabDragEnable() const;
	void EnableDropOptimization(bool optimiz);   // true - use optimization layout of panes after Drag and Drop operation (some panes can be deleted or transformed).
	bool IsDropOptimizationEnable() const;
		// 
	void SetMinTargetSizeForDrop(CSize const &size);   // Drag and Drop operation is allowed only if the size of the target pane under the cursor equal or more than 'size'.
	CSize GetMinTargetSizeForDrop() const;
		// 
	void SetLinePortions(HPANE parent, int *percent/*in*/);   // set proportions of child panes in the line 'parent' in percents (number items in 'percent'==GetNumberPanes(parent) and total number of percents must be 100).
	CRect GetPaneRect(HPANE pane) const;   // obtain the coordinates of the 'pane'. 
	void SetPanePortion(HPANE pane, double portion, Space space) const;
	double GetPanePortion(HPANE pane) const;   // obtain the part of the 'pane' in the parent line in the range (0.0,1.0].
	void CopyLinePortions(HPANE dst, MultiPaneCtrl const *paneCtrlSrc, HPANE src);   // copy proportions of child panes from 'src' of 'paneCtrlSrc' control to 'dst' of this control.
		// 
	void SetLineEqualPanesSize(HPANE parent);   // set equal size (width for horizontal line and height for vertical line) for all child panes of 'parent'.
	void SetLinesEqualPanesSize();   // set equal size (width for horizontal line and height for vertical line) for all child panes of every line in the control.
		// 
	void SetPanesMinSize(CSize const &size);   // set min width and height of panes in the control.
	CSize GetPanesMinSize() const;
		// 
	void WatchCtrlActivity(bool watch);   // true - control monitors the activity of child windows.
	bool IsWatchCtrlActivity() const;
		// 
	bool IsActive() const;   // return true - one of child windows is active (has focus), works only if IsWatchCtrlActivity()==true.
		// 
		// 
		// work with splitters.
	enum SplitterDragging { SplitterDraggingStatic, SplitterDraggingDynamic };
	void SetSplitterDraggingMode(SplitterDragging mode);
	SplitterDragging GetSplitterDraggingMode() const;
		// 
	bool IsSplitterDragging(bool *horz/*out,or null*/) const;
	void CancelDragging();
		// 
	void ActivateSplitter(HPANE pane, bool active);   // set activity for splitter which is right/bottom of pane.
	bool IsSplitterActive(HPANE pane) const;
		// 
	CRect GetSplitterRect(HPANE pane) const;   // splitter which is right/bottom of pane.
		// 
		// 
		// for getting information about tabs during load state operation.
	interface ITab
	{	virtual HWND GetWnd() const = 0;
		virtual CString GetText() const = 0;
		virtual int GetImage() const = 0;
		virtual CString GetToolTipText() const = 0;
		virtual bool IsDisable() const = 0;
		virtual __int64 GetData() const = 0;
	};
	struct Tabs : TOKEN_OWNER
	{	Tabs();
		~Tabs();
		int Add(HWND wnd, TCHAR const *text, int image);   // 'image'=-1 for tab without image.
		int Add(HWND wnd, TCHAR const *text, int image, TCHAR const *tooltip, bool disable, __int64 data);   // 'image'=-1 for tab without image.
		void DeleteAll();
		ITab const *operator[](int idx) const;
		int GetNumber() const;
	};
		// 
		// 
		// work with tabs.
	TabCtrl::HTAB AddTab(HPANE pane, HWND wnd, TCHAR const *text, int image);   // 'image'=-1 for tab without image.
	TabCtrl::HTAB AddTab(HPANE pane, ITab const *tab);
	TabCtrl::HTAB InsertTab(HPANE pane, TabCtrl::HTAB before, HWND wnd, TCHAR const *text, int image);   // 'image'=-1 for tab without image.
	TabCtrl::HTAB InsertTab(HPANE pane, TabCtrl::HTAB before, ITab const *tab);
		// 
	TabCtrl *GetTabCtrl(HPANE pane) const;   // only for a pane that is a line, i.e. which has no child panes.
	HPANE GetPaneWithTabCtrl(TabCtrl const *ctrl) const;
	HPANE GetPaneWithTab(TabCtrl::HTAB tab) const;
	TabCtrl::HTAB GetTabWithWindowID(int id, HPANE *pane/*out,or null*/) const;   // get tab whose window has 'id'.
	bool IsSinglePaneWithTabs() const;   // return true - there is only one pane with tabs.
	HPANE GetFirstPaneWithTabs() const;   // recursive search from GetFirst() to GetLast().
		// 
	void ActivateTab(HPANE pane);   // set focus into selected tab in this pane.
	void ActivateTab(HPANE pane, TabCtrl::HTAB tab);   // select and set focus into 'tab'.
		// 
	void SetTabsLayout(TabCtrl::Layout layout);
	TabCtrl::Layout GetTabsLayout() const;
	void SetTabsBehavior(TabCtrl::Behavior behavior);
	TabCtrl::Behavior GetTabsBehavior() const;
		// 
	void SetTabsScrollingStep(int step);   // width of one step for scrolling (in pixels >=1) (only for the TAB_BEHAVIOR_SCROLL mode).
	int GetTabsScrollingStep() const;
		// 
	void ShowTabsBorder(bool show);   // border is visible if IsTabsBorderVisible()==true and MultiPaneCtrl::IRecalc::GetBorderWidth(...) returns >0.
	bool IsTabsBorderVisible() const;
	void EqualTabsSize(bool equal);   // true - all tabs will have the same width.
	bool IsEqualTabsSize() const;
	void EnableTabRemove(bool enable);   // true - you can change positions of tabs using mouse.
	bool IsTabRemoveEnable() const;
	void HideSingleTab(bool hide);   // true - hide control area of TabCtrl if it has one tab. 
	bool IsHideSingleTab() const;
		// 
	void ShowButtonsClose(bool show);
	bool IsButtonsCloseVisible() const;
	void ShowButtonsMenu(bool show);
	bool IsButtonsMenuVisible() const;
	void ShowButtonsScroll(bool show);
	bool IsButtonsScrollVisible() const;
		// 
	void EnableTabsToolTip(bool enable);
	bool IsTabsToolTipEnable() const;
	void SetButtonsCloseToolTipText(TCHAR const *text);
	CString GetButtonsCloseToolTipText() const;
	void SetButtonsMenuToolTipText(TCHAR const *text);
	CString GetButtonsMenuToolTipText() const;
	void SetButtonsScrollLeftToolTipText(TCHAR const *text);
	CString GetButtonsScrollLeftToolTipText() const;
	void SetButtonsScrollRightToolTipText(TCHAR const *text);
	CString GetButtonsScrollRightToolTipText() const;
		// 
		// 
		// save/load state.
	bool GetAllTabs(Tabs *tabs/*out*/) const;   // get information about tabs exist in the control.
	bool LoadState(CWinApp *app, TCHAR const *section, TCHAR const *entry, Tabs const *tabs, bool ignoreLack);   // load state from registry.
	bool LoadStateAndUpdate(CWinApp *app, TCHAR const *section, TCHAR const *entry, Tabs const *tabs, bool ignoreLack);   // load state from registry + update control; makes panes refresh after loading faster.
	bool SaveState(CWinApp *app, TCHAR const *section, TCHAR const *entry) const;   // save state in registry.
	bool LoadState(CArchive *ar, Tabs const *tabs, bool ignoreLack);
	bool LoadStateAndUpdate(CArchive *ar, Tabs const *tabs, bool ignoreLack);   // load state + update control; makes panes refresh after loading faster.
	bool SaveState(CArchive *ar) const;
		// ignoreLack=true - ignore and not to load the pane and continue to load state if there is information about pane in the registry/archive, 
		//   but not in the 'tabs'; otherwise loading error.

public:   // functions of MultiPaneCtrl::IRecalc interface, return information from current recalc manager.
	int GetBorderWidth() const;
	CSize GetSplitterSize() const;

public:
	HPANE GetDragDstPane(TabCtrl::HTAB tabSrc/*or null*/, CPoint ptScr);   // get destination pane for drag operation.
	void DeleteOptimizDown(HPANE pane);   // recursive removal of the pane down to the root with optimization layout of panes.

///////////////////////////////////////
// PRIVATE
///////////////////////////////////////
private:
	struct Private;
	Private &p;

///////////////////////////////////////
// PROTECTED
///////////////////////////////////////
protected:
	DECLARE_MESSAGE_MAP()
	BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD style, const RECT &rect, CWnd *parentWnd, UINT nID, CCreateContext *context) override;
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) override;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct MultiPaneCtrlDrawBase : MultiPaneCtrl::Draw
{		// MultiPaneCtrl::Draw.
	void DrawSplitter(MultiPaneCtrl const *paneCtrl, CDC *dc, bool horz, CRect const *rect) override;
	void DrawSplitterDragRect(MultiPaneCtrl const *paneCtrl, CDC *dc, bool horz, CRect const *rectNew, CRect const *rectOld) override;
	void DrawBorder(MultiPaneCtrl const *paneCtrl, CDC *dc, CRect const *rect) override;

	virtual COLORREF GetSplitterColor(MultiPaneCtrl const *paneCtrl);
	virtual COLORREF GetBorderColor(MultiPaneCtrl const *paneCtrl);
	virtual void DrawDragRect(MultiPaneCtrl const *paneCtrl, CDC *dc, CRect const *rect);
	virtual CBrush *GetDragBrush();
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct MultiPaneCtrlStyle_base : MultiPaneCtrl::IStyle, 
	private TabCtrlStyle_base,
	private DockingMarkersDrawBase,
	private MultiPaneCtrlDrawBase
{
		// MultiPaneCtrl::IStyle.
	ITabCtrlStyle *GetTabCtrlStyle() override { return this; }
	DockingMarkers::Draw *GetDockMarkersDrawManager() override { return this; }
	MultiPaneCtrl::Draw *GetDrawManager() override { return this; }
	MultiPaneCtrl::IRecalc *GetRecalcManager() override { return nullptr; }   // use default manager.
};
/////////////////////////////////////////////////////////////////////////////
// 
template<typename TABSTYLE> struct MultiPaneCtrlStyle_VS2003_base : MultiPaneCtrl::IStyle, 
	private TABSTYLE, 
	private DockingMarkersDraw_VS2003,
	private MultiPaneCtrlDrawBase
{
		// MultiPaneCtrl::IStyle.
	ITabCtrlStyle *GetTabCtrlStyle() override { return this; }
	DockingMarkers::Draw *GetDockMarkersDrawManager() override { return this; }
	MultiPaneCtrl::Draw *GetDrawManager() override { return this; }
	MultiPaneCtrl::IRecalc *GetRecalcManager() override { return nullptr; }   // use default manager.
};
// 
typedef MultiPaneCtrlStyle_VS2003_base<TabCtrlStyle_VS2003_client> MultiPaneCtrlStyle_VS2003_client;
typedef MultiPaneCtrlStyle_VS2003_base<TabCtrlStyle_VS2003_client_custom1> MultiPaneCtrlStyle_VS2003_client_custom1;
typedef MultiPaneCtrlStyle_VS2003_base<TabCtrlStyle_VS2003_bars> MultiPaneCtrlStyle_VS2003_bars;
typedef MultiPaneCtrlStyle_VS2003_base<TabCtrlStyle_VS2003_bars_custom1> MultiPaneCtrlStyle_VS2003_bars_custom1;
// 
/////////////////////////////////////////////////////////////////////////////
// 
template<typename TABSTYLE> struct MultiPaneCtrlStyle_VS2008_base : MultiPaneCtrl::IStyle,
	private TABSTYLE,
	private DockingMarkersDraw_VS2008,
	private MultiPaneCtrlDrawBase
{
		// MultiPaneCtrl::IStyle.
	ITabCtrlStyle *GetTabCtrlStyle() override { return this; }
	DockingMarkers::Draw *GetDockMarkersDrawManager() override { return this; }
	MultiPaneCtrl::Draw *GetDrawManager() override { return this; }
	MultiPaneCtrl::IRecalc *GetRecalcManager() override { return nullptr; }   // use default manager.
};
// 
typedef MultiPaneCtrlStyle_VS2008_base<TabCtrlStyle_VS2008_client_classic> MultiPaneCtrlStyle_VS2008_client_classic;
typedef MultiPaneCtrlStyle_VS2008_base<TabCtrlStyle_VS2008_client_blue> MultiPaneCtrlStyle_VS2008_client_blue;
typedef MultiPaneCtrlStyle_VS2008_base<TabCtrlStyle_VS2008_client_silver> MultiPaneCtrlStyle_VS2008_client_silver;
typedef MultiPaneCtrlStyle_VS2008_base<TabCtrlStyle_VS2008_client_olive> MultiPaneCtrlStyle_VS2008_client_olive;
// 
typedef MultiPaneCtrlStyle_VS2008_base<TabCtrlStyle_VS2008_bars_classic> MultiPaneCtrlStyle_VS2008_bars_classic;
typedef MultiPaneCtrlStyle_VS2008_base<TabCtrlStyle_VS2008_bars_blue> MultiPaneCtrlStyle_VS2008_bars_blue;
typedef MultiPaneCtrlStyle_VS2008_base<TabCtrlStyle_VS2008_bars_silver> MultiPaneCtrlStyle_VS2008_bars_silver;
typedef MultiPaneCtrlStyle_VS2008_base<TabCtrlStyle_VS2008_bars_olive> MultiPaneCtrlStyle_VS2008_bars_olive;
// 
typedef MultiPaneCtrlStyle_VS2008_base<TabCtrlStyle_VS2008_bars_classic_custom1> MultiPaneCtrlStyle_VS2008_bars_classic_custom1;
typedef MultiPaneCtrlStyle_VS2008_base<TabCtrlStyle_VS2008_bars_blue_custom1> MultiPaneCtrlStyle_VS2008_bars_blue_custom1;
typedef MultiPaneCtrlStyle_VS2008_base<TabCtrlStyle_VS2008_bars_silver_custom1> MultiPaneCtrlStyle_VS2008_bars_silver_custom1;
typedef MultiPaneCtrlStyle_VS2008_base<TabCtrlStyle_VS2008_bars_olive_custom1> MultiPaneCtrlStyle_VS2008_bars_olive_custom1;
// 
/////////////////////////////////////////////////////////////////////////////
// 
template<typename TABSTYLE> struct MultiPaneCtrlStyle_VS2010_base : MultiPaneCtrl::IStyle, 
	private TABSTYLE,
	private DockingMarkersDraw_VS2010,
	private MultiPaneCtrlDrawBase,
	private MultiPaneCtrl::IRecalc
{
		// MultiPaneCtrl::IStyle.
	ITabCtrlStyle *GetTabCtrlStyle() override { return this; }
	DockingMarkers::Draw *GetDockMarkersDrawManager() override { return this; }
	MultiPaneCtrl::Draw *GetDrawManager() override { return this; }
	MultiPaneCtrl::IRecalc *GetRecalcManager() override { return this; }

		// MultiPaneCtrl::IRecalc.
	int GetBorderWidth(MultiPaneCtrl const *paneCtrl, MultiPaneCtrl::IRecalc *base) override { return base->GetBorderWidth(paneCtrl,nullptr); }
	CSize GetSplitterSize(MultiPaneCtrl const * /*paneCtrl*/, MultiPaneCtrl::IRecalc * /*base*/) override { return CSize(6,6); }

		// MultiPaneCtrlDrawBase.
	COLORREF GetSplitterColor(MultiPaneCtrl const * /*paneCtrl*/) override { return RGB(46,64,94); }
	COLORREF GetBorderColor(MultiPaneCtrl const * /*paneCtrl*/) override { return RGB(46,64,94); }
	CBrush *GetDragBrush() override { static CBrush br(~::GetSysColor(COLOR_BTNSHADOW) & 0x00ffffff); return &br; }
};
// 
typedef MultiPaneCtrlStyle_VS2010_base<TabCtrlStyle_VS2010_client> MultiPaneCtrlStyle_VS2010_client;
typedef MultiPaneCtrlStyle_VS2010_base<TabCtrlStyle_VS2010_bars> MultiPaneCtrlStyle_VS2010_bars;
//
/////////////////////////////////////////////////////////////////////////////
// 
template<typename TABSTYLE, COLORREF SPLITTERCOLOR, COLORREF BORDERCOLOR> struct MultiPaneCtrlStyle_VS2019_base : MultiPaneCtrl::IStyle, 
	private TABSTYLE,
	private DockingMarkersDraw_VS2010,
	private MultiPaneCtrlDrawBase,
	private MultiPaneCtrl::IRecalc
{
		// MultiPaneCtrl::IStyle.
	ITabCtrlStyle *GetTabCtrlStyle() override { return this; }
	DockingMarkers::Draw *GetDockMarkersDrawManager() override { return this; }
	MultiPaneCtrl::Draw *GetDrawManager() override { return this; }
	MultiPaneCtrl::IRecalc *GetRecalcManager() override { return this; }

		// MultiPaneCtrl::IRecalc.
	int GetBorderWidth(MultiPaneCtrl const *paneCtrl, MultiPaneCtrl::IRecalc *base) override { return base->GetBorderWidth(paneCtrl,nullptr); }
	CSize GetSplitterSize(MultiPaneCtrl const * /*paneCtrl*/, MultiPaneCtrl::IRecalc * /*base*/) override { return CSize(6,6); }

		// MultiPaneCtrlDrawBase.
	COLORREF GetSplitterColor(MultiPaneCtrl const * /*paneCtrl*/) override { return SPLITTERCOLOR; }
	COLORREF GetBorderColor(MultiPaneCtrl const * /*paneCtrl*/) override { return BORDERCOLOR; }
	CBrush *GetDragBrush() override { static CBrush br(0x00919191); return &br; }};
// 
typedef MultiPaneCtrlStyle_VS2019_base<TabCtrlStyle_VS2019_client_light,RGB(238,238,242),RGB(204,206,219)> MultiPaneCtrlStyle_VS2019_client_light;
typedef MultiPaneCtrlStyle_VS2019_base<TabCtrlStyle_VS2019_client_light_custom1,RGB(238,238,242),RGB(204,206,219)> MultiPaneCtrlStyle_VS2019_client_light_custom1;
typedef MultiPaneCtrlStyle_VS2019_base<TabCtrlStyle_VS2019_client_dark,RGB(45,45,48),RGB(63,63,70)> MultiPaneCtrlStyle_VS2019_client_dark;
typedef MultiPaneCtrlStyle_VS2019_base<TabCtrlStyle_VS2019_client_blue,RGB(93,107,153),RGB(93,107,153)> MultiPaneCtrlStyle_VS2019_client_blue;
// 
typedef MultiPaneCtrlStyle_VS2019_base<TabCtrlStyle_VS2019_bars_light,RGB(238,238,242),RGB(204,206,219)> MultiPaneCtrlStyle_VS2019_bars_light;
typedef MultiPaneCtrlStyle_VS2019_base<TabCtrlStyle_VS2019_bars_dark,RGB(45,45,48),RGB(63,63,70)> MultiPaneCtrlStyle_VS2019_bars_dark;
typedef MultiPaneCtrlStyle_VS2019_base<TabCtrlStyle_VS2019_bars_blue,RGB(93,107,153),RGB(93,107,153)> MultiPaneCtrlStyle_VS2019_bars_blue;
// 
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
template<typename CLASS_STYLE>
struct MultiPaneCtrlEx : MultiPaneCtrl
{	MultiPaneCtrlEx()
	{	InstallStyle(&style);
	}
	CLASS_STYLE style;
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////











