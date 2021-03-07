//==========================================================
// Author: Baradzenka Aleh (baradzenka@gmail.com)
//==========================================================
// 
#pragma once
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#if (!defined(_MSC_VER) && __cplusplus < 201103L) || (defined(_MSC_VER) && _MSC_VER < 1900)   // C++11 is not supported.
	#define nullptr  NULL
	#define override
#endif
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct DockingMarkers
{
///////////////////////////////////////
// PUBLIC
///////////////////////////////////////
	enum Position
	{	PositionNone,
		PositionLeft, PositionTop, PositionRight, PositionBottom, PositionCenter
	};
	struct Draw
	{	virtual COLORREF GetInsertionWindowTranspColor(DockingMarkers * /*markers*/) = 0;
		virtual void DrawInsertionWindow(DockingMarkers * /*markers*/, CDC * /*dc*/, CRect const * /*rect*/, bool /*center*/, bool /*tabsOnTop*/, int /*tabHeight*/) {}
		virtual void DrawInsertionFrame(DockingMarkers * /*markers*/, CDC * /*dc*/, CRect const * /*rectNew*/, CRect const * /*rectOld*/, bool /*center*/, bool /*tabsOnTop*/, int /*tabHeight*/) {}   // uses when IsCreated()==false -> DockingMarkers wasn't created.
	};
	interface Ability
	{	virtual bool IsMarkerEnable(DockingMarkers * /*markers*/, Position /*position*/) { return true; }
	};

public:
	struct Marker
	{	Marker();
		Marker(POINT const &pos, UINT resID, RECT const &area=CRect(0,0,0,0));
		const CPoint pos;   // position of marker's image.
		const UINT resID;   // ID of bitmap or png in resources.
		const CRect area;   // sensitive area (relative to the upper left corner of this marker).
	};
	struct Layout   // layout for markers.
	{	Layout(int widthTotal, int heightTotal,
			Marker const &left, Marker const &top, Marker const &right, Marker const &bottom, Marker const &center, Marker const &back, 
			HMODULE moduleRes/*or null*/, bool pngImage, COLORREF clrMask,
			int transp, bool animation, int animationStep);
			// 
		const int widthTotal, heightTotal;   // total size of marker's group.
		const Marker left, top, right, bottom, center, back;
			// 
		HMODULE moduleRes;   // module containing resources (bitmaps or pngs).
		const bool pngImage;   // true if Marker::resID points to png.
		const COLORREF clrMask;   // color of not visible pixels in Marker::resID.
			// 
		const int transparence;   // 0...99% - transparence of markers.
		const bool animation;   // enable/disable animation.
		const int animationStep;   // 1...100% - step of transparence changing during animation.
	};

///////////////////////////////////////
// PRIVATE
///////////////////////////////////////
private:
	friend class MultiPaneCtrl;
		// 
	DockingMarkers();
	~DockingMarkers();
		// 
	bool Create(Layout const &layout, int insertMarkerTransp/*1...100%*/);
	void Destroy();
	bool IsCreated() const;
		// 
	void ShowMarkers(CRect const *rcScr);
	void HideMarkers();
		// 
	void ShowInsertionArea(CWnd *parentWnd, CRect const *rcScr, bool center, bool tabsOnTop, int tabHeight);
	void HideInsertionArea(CWnd *parentWnd);
		// 
	Position RelayMouseMove(CPoint pt);
		// 
	void SetAbilityManager(Ability *p/*or null*/);   // null for default manager.
	Ability *GetAbilityManager() const;
	void SetDrawManager(Draw *p/*or null*/);
	Draw *GetDrawManager() const;

private:
	struct Private;
	Private &p;
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
struct DockingMarkersDrawBase : DockingMarkers::Draw
{		// DockingMarkers::Draw.
	COLORREF GetInsertionWindowTranspColor(DockingMarkers * /*markers*/) override;
	void DrawInsertionWindow(DockingMarkers *markers, CDC *dc, CRect const *rect, bool center, bool tabsOnTop, int tabHeight) override;
	void DrawInsertionFrame(DockingMarkers *markers, CDC *dc, CRect const *rectNew, CRect const *rectOld, bool center, bool tabsOnTop, int tabHeight) override;   // uses when IsCreated()==false -> DockingMarkers wasn't created.

	virtual int GetInsertionWindowBorderWidth(DockingMarkers *markers);
	virtual int GetInsertionFrameWidth(DockingMarkers *markers);

	virtual COLORREF GetInsertionWindowInsideColor(DockingMarkers *markers);
	virtual COLORREF GetInsertionWindowBorderColor(DockingMarkers *markers);

	virtual void DrawInsertionWindowInside(DockingMarkers *markers, CDC *dc, CRect const *rect);
	virtual void DrawInsertionWindowBorder(DockingMarkers *markers, CDC *dc, CRect const *rect);
	virtual void DrawInsertionFrame(DockingMarkers *markers, CDC *dc, CRect const *rect);

	virtual void GetOuterOutline(DockingMarkers *markers, CRect const *rect, bool top, int tabHeight, int width, POINT pts[8]/*out*/);
	virtual void GetInnerOutline(DockingMarkers *markers, CRect const *rect, bool top, int tabHeight, int width, POINT pts[8]/*out*/);
	void FillSolidRect(CDC *dc, CRect const *rc, COLORREF color) const;
};
/////////////////////////////////////////////////////////////////////////////
//
struct DockingMarkersDraw_custom1 : DockingMarkersDrawBase
{	int GetInsertionWindowBorderWidth(DockingMarkers *markers) override;
	int GetInsertionFrameWidth(DockingMarkers *markers) override;

	COLORREF GetInsertionWindowBorderColor(DockingMarkers *markers) override;

	void DrawInsertionWindowInside(DockingMarkers *markers, CDC *dc, CRect const *rect) override;
	void DrawInsertionFrame(DockingMarkers *markers, CDC *dc, CRect const *rect) override;
};
/////////////////////////////////////////////////////////////////////////////
//
struct DockingMarkersDraw_VS2003 : DockingMarkersDrawBase
{	int GetInsertionFrameWidth(DockingMarkers * /*markers*/) override { return 8; }
};
/////////////////////////////////////////////////////////////////////////////
// 
struct DockingMarkersDraw_VS2005 : DockingMarkersDrawBase
{	int GetInsertionWindowBorderWidth(DockingMarkers * /*markers*/) override { return 2; }
};
/////////////////////////////////////////////////////////////////////////////
// 
struct DockingMarkersDraw_VS2008 : DockingMarkersDrawBase
{
};
/////////////////////////////////////////////////////////////////////////////
//
struct DockingMarkersDraw_VS2010 : DockingMarkersDrawBase
{	int GetInsertionWindowBorderWidth(DockingMarkers *markers) override;
	COLORREF GetInsertionWindowInsideColor(DockingMarkers *markers) override;

	void GetOuterOutline(DockingMarkers *markers, CRect const *rect, bool top, int tabHeight, int width, POINT pts[8]/*out*/) override;
	void GetInnerOutline(DockingMarkers *markers, CRect const *rect, bool top, int tabHeight, int width, POINT pts[8]/*out*/) override;
	COLORREF MixingColors(COLORREF src, COLORREF dst, int percent) const;
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////













