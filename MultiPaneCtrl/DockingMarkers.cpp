//==========================================================
// Author: Baradzenka Aleh (baradzenka@gmail.com)
//==========================================================
// 
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DockingMarkers.h"
#include <cassert>
#include <list>
#include <algorithm>
// 
#pragma warning(push)
#pragma warning(disable : 4458)   // declaration of 'nativeCap' hides class member.
	#include <gdiplus.h>
#pragma warning(pop)
// 
#pragma warning(push)
#pragma warning(disable : 4201)   // nonstandard extension used.
	#include <Mmsystem.h>
#pragma warning(pop)
#pragma comment (lib, "Winmm.lib")   // for Multimedia Timers.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4355)   // 'this' : used in base member initializer list.
#undef max
#undef min
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct DockingMarkers::Private : 
	DockingMarkers::Ability
{
	struct VirtualWindow : CDC
	{	VirtualWindow(CWnd *wnd)
		{	assert(wnd && ::IsWindow(wnd->m_hWnd));
			pwnd = wnd;
			pdc = pwnd->BeginPaint(&ps/*out*/);
			pwnd->GetClientRect(&rect/*out*/);
			if(CreateCompatibleDC(pdc) && bitmap.CreateCompatibleBitmap(pdc,rect.Width(),rect.Height()))
			{	SelectObject(&bitmap);
				SetBkMode(TRANSPARENT);
			}
		}
		~VirtualWindow()
		{	if(bitmap.m_hObject)
				pdc->BitBlt(0,0,rect.Width(),rect.Height(), this, 0,0, SRCCOPY);
			pwnd->EndPaint(&ps);
		}

	private:
		CWnd *pwnd;
		PAINTSTRUCT ps;
		CDC *pdc;
		CRect rect;
		CBitmap bitmap;
	};
		// 
	struct Timer : private CWnd
	{	interface INotify
		{	virtual void OnTimer(Timer *timer, int id, int time) = 0;
		};

	public:
		Timer()
		{	m_pINotify = nullptr;
			m_iID = m_iTime = 0;
			m_uTimerID = 0;
		}
		~Timer()
		{	KillTimer();
		}
			// 
		bool SetTimer(INotify *p, int id, int time/*ms*/)
		{	assert(p);
			assert(id>0 && time>0);
				// 
			if(m_hWnd)
				KillTimer();
				// 
			m_pINotify = p;
			m_iID = id;
			m_iTime = time;
				// 
			if(!CWnd::Create(AfxRegisterWndClass(CS_DBLCLKS),_T(""),WS_CHILD,CRect(0,0,0,0),CWnd::GetDesktopWindow(),100) ||
				(m_uTimerID = timeSetEvent(time,0,TimeProc,reinterpret_cast<DWORD_PTR>(m_hWnd),TIME_PERIODIC))==0)
			{
				KillTimer();
				return false;
			}
			return true;
		}
		void KillTimer()
		{	if(m_uTimerID)
			{	timeKillEvent(m_uTimerID);
				m_uTimerID = 0;
			}
			if(m_hWnd)
				DestroyWindow();
				//  
			m_pINotify = nullptr;
			m_iID = m_iTime = 0;
		}

	protected:
		INotify *m_pINotify;
		int m_iID, m_iTime;
			// 
		UINT m_uTimerID;

	protected:
		static void CALLBACK TimeProc(UINT /*uID*/, UINT /*uMsg*/, DWORD_PTR dwUser, DWORD_PTR /*dw1*/, DWORD_PTR /*dw2*/)
		{	HWND wnd = reinterpret_cast<HWND>(dwUser);
			::SendMessage(wnd,WM_USER,0,0);
		}
			// 
		LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override
		{	const LRESULT res = CWnd::WindowProc(message,wParam,lParam);
			if(message==WM_USER)
				m_pINotify->OnTimer(this,m_iID,m_iTime);
			return res;
		}
	};

public:
	Private(DockingMarkers &owner);

private:
	DockingMarkers &o;

public:
	struct MarkerWindow : CWnd
	{	MarkerWindow(DockingMarkers *p, Position position);
		~MarkerWindow();
			// 
		bool Create(HMODULE moduleRes/*or null*/, UINT resID, bool pngImage, COLORREF clrMask/*or CLR_NONE*/);
		void Destroy();
			// 
		void SetPos(POINT pt);
		void Disable(bool disable);
		void Highlight(bool highlight);
		bool IsHighlight() const;
		void SetTransparence(int transparence/*percents*/);

	private:
		DockingMarkers *m_pDockingMarkers;
		const Position m_Position;
			// 
		CBitmap m_Image;
		COLORREF m_clrMask;
			// 
		bool m_bDisable;
		bool m_bHighlight;
			// 
		BOOL(__stdcall *m_pUpdateLayeredWindow)(HWND,HDC,POINT*,SIZE*,HDC,POINT*,COLORREF,BLENDFUNCTION*,DWORD);
	};
		// 
	struct Animation : Timer::INotify
	{	void Init(Layout const &layout);
		bool Add(MarkerWindow *wnd);
		void Destroy();
			// 
		void Show();
		void Hide();
		void Highlight(MarkerWindow *wnd, bool highlight);

	private:   // Timer::INotify.
		void OnTimer(Timer *timer, int id, int time) override;

	private:
		Timer m_Timer;
			// 
		int m_iBaseTransparence;
		bool m_bAnimation;
		int m_iAnimationStep;
			// 
		int m_iTransparence;
		std::list<MarkerWindow *> m_MarkerWindows;
		typedef std::list<MarkerWindow *>::iterator i_markerWindows;
	};
		// 
	struct MarkerGroup
	{	MarkerGroup(DockingMarkers *p);
		MarkerWindow left, top, right, bottom, center, back;
		Animation animation;
		enum Type { TypeFirst, TypeSecond };
		CPoint basePoint;
	} m_MarkerGroup1, m_MarkerGroup2;
	MarkerGroup::Type m_ShownGroup;
		// 
	Ability *m_pAbilityManager;
		// 
	Layout m_Layout;
		// 
	bool m_bCenter, m_bTabsOnTop;
	int m_iTabHeight;
		// 
	struct InsertionWindow : CWnd
	{	InsertionWindow(DockingMarkers *p);
		~InsertionWindow();
			// 
		bool Create(int transparence);
		void Destroy();
			// 
		void SetParams(bool center, bool tabsOnTop, int tabHeight);
		void Show(HWND hWndAfter, CRect const *rcScr);
		void Hide();
			// 
		void SetDrawManager(Draw *p);
		Draw *GetDrawManager() const;

	private:
		DockingMarkers *m_pDockingMarkers;
			// 
		Draw *m_pDrawManager;
		bool m_bCenter, m_bTabsOnTop;
		int m_iTabHeight;
			//
		BOOL( __stdcall *m_pSetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);
			// 
		DECLARE_MESSAGE_MAP()
		LRESULT OnPaint(WPARAM wp, LPARAM lp);
	} m_InsertWindow;

public:
	MarkerGroup *GetWorkingMarkerGroup();
	MarkerGroup *GetNotWorkingMarkerGroup();
	Position HitTest(CPoint pt);
	bool CreateMarkerWindows(Layout const &layout);
	void DrawInsertionFrame(CWnd *parentWnd, CRect const *rect);
	static bool LoadImage(HMODULE moduleRes/*or null*/, UINT resID, bool pngImage, Gdiplus::Bitmap **bmp/*out*/);
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Marker::Marker() :
	pos(CPoint(0,0)), resID(0), area(CRect(0,0,0,0))
{
}
// 
DockingMarkers::Marker::Marker(POINT const &pos, UINT resID, RECT const &area/*=CRect(0,0,0,0)*/) :
	pos(pos), resID(resID), area(area)
{
}
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Layout::Layout(int widthTotal, int heightTotal,
	Marker const &left, Marker const &top, Marker const &right, Marker const &bottom, Marker const &center, Marker const &back, 
	HMODULE moduleRes/*or null*/, bool pngImage, COLORREF clrMask,
	int transp, bool animation, int animationStep)
:
	widthTotal(widthTotal), heightTotal(heightTotal), 
	left(left), top(top), right(right), bottom(bottom), center(center), back(back),
	moduleRes(moduleRes), pngImage(pngImage), clrMask(clrMask),
	transparence(transp), animation(animation), animationStep(animationStep)
{
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkers::MarkerWindow.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Private::MarkerWindow::MarkerWindow(DockingMarkers *p, Position position) : 
	m_pDockingMarkers(p), m_Position(position)
{
	HMODULE hUser32 = LoadLibrary(_T("User32.dll"));
	m_pUpdateLayeredWindow = reinterpret_cast<BOOL(__stdcall *)(HWND,HDC,POINT*,SIZE*,HDC,POINT*,COLORREF,BLENDFUNCTION*,DWORD)>( ::GetProcAddress(hUser32,"UpdateLayeredWindow") );
}
// 
DockingMarkers::Private::MarkerWindow::~MarkerWindow()
{	DestroyWindow();   // for delete warning: 'calling DestroyWindow in CWnd::~CWnd; OnDestroy or PostNcDestroy in derived class will not be called'.
}
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::Private::MarkerWindow::Create(HMODULE moduleRes/*or null*/, UINT resID, bool pngImage, COLORREF clrMask/*or CLR_NONE*/)
{	if(resID==0)   // this marker doesn't use.
		return true;
		// 
	if(!m_pUpdateLayeredWindow)
		return false;
		// 
	const CString className = AfxRegisterWndClass(CS_DBLCLKS,::LoadCursor(nullptr,IDC_ARROW),nullptr,nullptr);
	if( !CWnd::CreateEx(WS_EX_TOOLWINDOW | 0x00080000/*WS_EX_LAYERED*/,className,_T(""),WS_POPUP,CRect(0,0,0,0),nullptr,0) )
		return false;
		// 
	Gdiplus::Bitmap *bmp;
	if( Private::LoadImage(moduleRes,resID,pngImage,&bmp/*out*/) )
	{	const Gdiplus::Rect rect(0,0,bmp->GetWidth(),bmp->GetHeight());
		Gdiplus::BitmapData data;
		if( bmp->LockBits(&rect,Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite,PixelFormat32bppPARGB,&data/*ou*/)==Gdiplus::Ok )
		{	m_Image.CreateBitmap(rect.Width,rect.Height,1,32,data.Scan0);
			bmp->UnlockBits(&data);
		}
		::delete bmp;
	}
	if(!m_Image.m_hObject)
		return false;
	m_clrMask = clrMask;
		// 
	BITMAP bmpData;
	m_Image.GetBitmap(&bmpData/*out*/);
	const int width = (m_Position==PositionNone/*background*/ ? bmpData.bmWidth : bmpData.bmWidth/3);
	::SetWindowPos(m_hWnd, nullptr, 0,0,width,bmpData.bmHeight, SWP_NOZORDER | SWP_NOMOVE);   // set size.
		// 
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::MarkerWindow::Destroy()
{	if(m_Image.m_hObject)
		m_Image.DeleteObject();
	if( ::IsWindow(m_hWnd) )
		::DestroyWindow(m_hWnd);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::MarkerWindow::SetPos(POINT pt)
{	if(m_hWnd)
		::SetWindowPos(m_hWnd, HWND_TOPMOST, pt.x,pt.y,0,0, SWP_NOSIZE | SWP_NOACTIVATE);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::MarkerWindow::Disable(bool disable)
{	m_bDisable = disable;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::MarkerWindow::Highlight(bool highlight)
{	m_bHighlight = highlight;
}
// 
bool DockingMarkers::Private::MarkerWindow::IsHighlight() const
{	return m_bHighlight;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::MarkerWindow::SetTransparence(int transparence/*percents*/)
{	if(m_hWnd)
	{	CClientDC dc(this);
		CDC dcComp;
		if( dcComp.CreateCompatibleDC(&dc) )
		{	CRect rect;
			GetWindowRect(&rect/*out*/);
				// 
			static BLENDFUNCTION func = {AC_SRC_OVER,0,0,AC_SRC_ALPHA};
			func.SourceConstantAlpha = static_cast<BYTE>(((100 - transparence) * 255) / 100);
				// 
			CSize szDst = rect.Size();
			const int image = (m_Position==PositionNone/*background*/ ? 0 : (!m_bDisable ? (!m_bHighlight ? 0 : 1) : 2));
			CPoint ptSrc(image*szDst.cx,0);
				// 
			CBitmap *bmpOld = static_cast<CBitmap*>( dcComp.SelectObject(&m_Image) );
			m_pUpdateLayeredWindow(m_hWnd, nullptr, nullptr,&szDst, dcComp.m_hDC, &ptSrc, m_clrMask, &func, 0x00000002/*ULW_ALPHA*/ | (m_clrMask!=CLR_NONE ? 0x00000001/*ULW_COLORKEY*/ : 0));
			dcComp.SelectObject(bmpOld);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkers::Animation.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::Animation::Init(Layout const &layout)
{	m_iBaseTransparence = layout.transparence;
	m_bAnimation = layout.animation;
	m_iAnimationStep = layout.animationStep;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::Animation::Destroy()
{	m_Timer.KillTimer();
	m_MarkerWindows.clear();
}
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::Private::Animation::Add(MarkerWindow *wnd)
{	try
	{	m_MarkerWindows.push_back(wnd);
	}
	catch(std::bad_alloc &)
	{	return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::Animation::Show()
{	i_markerWindows i, e=m_MarkerWindows.end();
		// 
	for(i=m_MarkerWindows.begin(); i!=e; ++i)
		(*i)->Highlight(false);
		// 
	if(!m_bAnimation)
		for(i=m_MarkerWindows.begin(); i!=e; ++i)
			(*i)->SetTransparence(m_iBaseTransparence);
	else
	{	m_Timer.KillTimer();
			// 
		for(i=m_MarkerWindows.begin(); i!=e; ++i)
			(*i)->SetTransparence(100);
			// 
		m_iTransparence = 100;
		m_Timer.SetTimer(this,1,40);
	}
		// 
	for(i=m_MarkerWindows.begin(); i!=e; ++i)
		if( !::IsWindowVisible((*i)->m_hWnd) )
		 ::SetWindowPos((*i)->m_hWnd, nullptr, 0,0,0,0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);   // show window.
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::Animation::Hide()
{	if( !::IsWindowVisible(m_MarkerWindows.front()->m_hWnd) )
		return;
		// 
	for(i_markerWindows i=m_MarkerWindows.begin(), e=m_MarkerWindows.end(); i!=e; ++i)
		if( (*i)->IsHighlight() )
			(*i)->Highlight(false);
		// 
	if(!m_bAnimation)
		for(i_markerWindows i=m_MarkerWindows.begin(), e=m_MarkerWindows.end(); i!=e; ++i)
			(*i)->SetTransparence(100);
	else
	{	m_Timer.KillTimer();
		m_Timer.SetTimer(this,2,40);
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::Animation::OnTimer(Timer *timer, int id, int /*time*/)
{	if(id==1)   // appearance.
	{	m_iTransparence = std::max(m_iBaseTransparence,m_iTransparence-m_iAnimationStep);
		if(m_iTransparence==m_iBaseTransparence)
			timer->KillTimer();
	}
	else   // disappearance.
	{	m_iTransparence = std::min(100,m_iTransparence+m_iAnimationStep);
		if(m_iTransparence==100)
			timer->KillTimer();
	}
		// 
	for(i_markerWindows i=m_MarkerWindows.begin(), e=m_MarkerWindows.end(); i!=e; ++i)
		if(id==1 && (*i)->IsHighlight() && m_iTransparence==m_iBaseTransparence)   // cursor above marker and it is finish of appearance.
			(*i)->SetTransparence(0);
		else
			(*i)->SetTransparence(m_iTransparence);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::Animation::Highlight(MarkerWindow *wnd, bool highlight)
{	if(highlight!=wnd->IsHighlight())
	{	wnd->Highlight(highlight);
		if(!m_bAnimation || m_iTransparence<=m_iBaseTransparence)
			wnd->SetTransparence(!highlight ? m_iBaseTransparence : 0);
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkers::MarkerGroup.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Private::MarkerGroup::MarkerGroup(DockingMarkers *p) :
	left(p,PositionLeft), top(p,PositionTop), 
	right(p,PositionRight), bottom(p,PositionBottom),
	center(p,PositionCenter), back(p,PositionNone)
{
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkers::InsertionWindow.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(DockingMarkers::Private::InsertionWindow, CWnd)
	ON_MESSAGE(WM_PAINT, DockingMarkers::Private::InsertionWindow::OnPaint)
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Private::InsertionWindow::InsertionWindow(DockingMarkers *p) :
	m_pDockingMarkers(p)
{
	m_pDrawManager = nullptr;
		// 
	HMODULE hUser32 = LoadLibrary(_T("User32.dll"));
	m_pSetLayeredWindowAttributes = reinterpret_cast<BOOL( __stdcall *)(HWND,COLORREF,BYTE,DWORD)>( ::GetProcAddress(hUser32,"SetLayeredWindowAttributes") );
}
// 
DockingMarkers::Private::InsertionWindow::~InsertionWindow()
{	DestroyWindow();   // for delete warning: 'calling DestroyWindow in CWnd::~CWnd; OnDestroy or PostNcDestroy in derived class will not be called'.
}
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::Private::InsertionWindow::Create(int transparence)
{	if(!m_pSetLayeredWindowAttributes)
		return false;
		// 
	const CString className = AfxRegisterWndClass(CS_DBLCLKS,::LoadCursor(nullptr,IDC_ARROW),nullptr,nullptr);
	if( !CWnd::CreateEx(WS_EX_TOOLWINDOW | 0x00080000/*WS_EX_LAYERED*/,className,_T(""),WS_POPUP,CRect(0,0,0,0),nullptr,0) )
		return false;
		// 
	const COLORREF clrMask = (m_pDrawManager ? m_pDrawManager->GetInsertionWindowTranspColor(m_pDockingMarkers) : RGB(255,0,255));
	const BYTE transp = static_cast<BYTE>(((100 - transparence) * 255) / 100);
	m_pSetLayeredWindowAttributes(m_hWnd,clrMask,transp,0x00000001/*LWA_COLORKEY*/ | 0x00000002/*LWA_ALPHA*/);
		// 
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::InsertionWindow::Destroy()
{	if( ::IsWindow(m_hWnd) )
		::DestroyWindow(m_hWnd);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::InsertionWindow::SetParams(bool center, bool tabsOnTop, int tabHeight)
{	m_bCenter = center;
	m_bTabsOnTop = tabsOnTop;
	m_iTabHeight = tabHeight;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::InsertionWindow::Show(HWND wndAfter, CRect const *rcScr)
{	::SetWindowPos(m_hWnd, wndAfter, rcScr->left,rcScr->top,rcScr->Width(),rcScr->Height(), 
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS);
	::RedrawWindow(m_hWnd,nullptr,nullptr,RDW_INVALIDATE | RDW_UPDATENOW);
}
// 
void DockingMarkers::Private::InsertionWindow::Hide()
{	::ShowWindow(m_hWnd,SW_HIDE);
}
/////////////////////////////////////////////////////////////////////////////
// 
LRESULT DockingMarkers::Private::InsertionWindow::OnPaint(WPARAM /*wp*/, LPARAM /*lp*/)
{	Private::VirtualWindow virtwnd(this);
	if( !virtwnd.GetSafeHdc() )
	{	CPaintDC dc(this);
		return 0;
	}
		// 
	if(m_pDrawManager)
	{	CRect rc;
		GetClientRect(&rc/*out*/);
			//
		HBRUSH hBrush = ::CreateSolidBrush( m_pDrawManager->GetInsertionWindowTranspColor(m_pDockingMarkers) );
		::FillRect(virtwnd.m_hDC,&rc,hBrush);
		::DeleteObject(hBrush);
			// 
		m_pDrawManager->DrawInsertionWindow(m_pDockingMarkers,&virtwnd,&rc,m_bCenter,m_bTabsOnTop,m_iTabHeight);
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::InsertionWindow::SetDrawManager(Draw *p)
{	m_pDrawManager = p;
}
// 
DockingMarkers::Draw *DockingMarkers::Private::InsertionWindow::GetDrawManager() const
{	return m_pDrawManager;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkers.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::DockingMarkers() :
	p( *new Private(*this) )
{
}
//
DockingMarkers::~DockingMarkers()
{	delete &p;
}
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Private::Private(DockingMarkers &owner) : o(owner),
	m_Layout(0,0,
		Marker(CPoint(0,0),0,CRect(0,0,0,0)), Marker(CPoint(0,0),0,CRect(0,0,0,0)),
		Marker(CPoint(0,0),0,CRect(0,0,0,0)), Marker(CPoint(0,0),0,CRect(0,0,0,0)), 
		Marker(CPoint(0,0),0,CRect(0,0,0,0)), Marker(CPoint(0,0),0,CRect(0,0,0,0)), 
		nullptr,false,CLR_NONE, 0,false,0),
	m_MarkerGroup1(&o), m_MarkerGroup2(&o), 
	m_InsertWindow(&o)
{
	m_pAbilityManager = this;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::Create(Layout const &layout, int insertMarkerTransp/*1...100%*/)
{	assert( !IsCreated() );
	assert(layout.transparence>=0 && layout.transparence<=99);
	assert(!layout.animation || (layout.animationStep>=1 && layout.animationStep<=100));
	assert(insertMarkerTransp>=1 && insertMarkerTransp<=100);
		// 
	if( !p.CreateMarkerWindows(layout) ||
		!p.m_InsertWindow.Create(insertMarkerTransp) )
	{
		Destroy();
		return false;
	}
		// 
	::CopyMemory(&p.m_Layout,&layout,sizeof(layout));
	p.m_ShownGroup = Private::MarkerGroup::TypeFirst;
		// 
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::Private::CreateMarkerWindows(Layout const &layout)
{	m_MarkerGroup1.animation.Init(layout);
	m_MarkerGroup2.animation.Init(layout);
		// 
	return 
		m_MarkerGroup1.animation.Add(&m_MarkerGroup1.left) && 
		m_MarkerGroup1.animation.Add(&m_MarkerGroup1.top) && 
		m_MarkerGroup1.animation.Add(&m_MarkerGroup1.right) && 
		m_MarkerGroup1.animation.Add(&m_MarkerGroup1.bottom) && 
		m_MarkerGroup1.animation.Add(&m_MarkerGroup1.center) && 
		m_MarkerGroup1.animation.Add(&m_MarkerGroup1.back) && 
			// 
		m_MarkerGroup2.animation.Add(&m_MarkerGroup2.left) && 
		m_MarkerGroup2.animation.Add(&m_MarkerGroup2.top) && 
		m_MarkerGroup2.animation.Add(&m_MarkerGroup2.right) && 
		m_MarkerGroup2.animation.Add(&m_MarkerGroup2.bottom) && 
		m_MarkerGroup2.animation.Add(&m_MarkerGroup2.center) && 
		m_MarkerGroup2.animation.Add(&m_MarkerGroup2.back) && 
			// 
		m_MarkerGroup1.left.Create(layout.moduleRes,layout.left.resID,layout.pngImage,layout.clrMask) && 
		m_MarkerGroup1.top.Create(layout.moduleRes,layout.top.resID,layout.pngImage,layout.clrMask) &&
		m_MarkerGroup1.right.Create(layout.moduleRes,layout.right.resID,layout.pngImage,layout.clrMask) && 
		m_MarkerGroup1.bottom.Create(layout.moduleRes,layout.bottom.resID,layout.pngImage,layout.clrMask) && 
		m_MarkerGroup1.center.Create(layout.moduleRes,layout.center.resID,layout.pngImage,layout.clrMask) &&
		m_MarkerGroup1.back.Create(layout.moduleRes,layout.back.resID,layout.pngImage,layout.clrMask) &&
			// 
		m_MarkerGroup2.left.Create(layout.moduleRes,layout.left.resID,layout.pngImage,layout.clrMask) && 
		m_MarkerGroup2.top.Create(layout.moduleRes,layout.top.resID,layout.pngImage,layout.clrMask) &&
		m_MarkerGroup2.right.Create(layout.moduleRes,layout.right.resID,layout.pngImage,layout.clrMask) && 
		m_MarkerGroup2.bottom.Create(layout.moduleRes,layout.bottom.resID,layout.pngImage,layout.clrMask) && 
		m_MarkerGroup2.center.Create(layout.moduleRes,layout.center.resID,layout.pngImage,layout.clrMask) &&
		m_MarkerGroup2.back.Create(layout.moduleRes,layout.back.resID,layout.pngImage,layout.clrMask);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Destroy()
{	p.m_MarkerGroup1.animation.Destroy();
	p.m_MarkerGroup2.animation.Destroy();
		// 
	p.m_MarkerGroup1.left.Destroy();
	p.m_MarkerGroup1.top.Destroy();
	p.m_MarkerGroup1.right.Destroy();
	p.m_MarkerGroup1.bottom.Destroy();
	p.m_MarkerGroup1.center.Destroy();
	p.m_MarkerGroup1.back.Destroy();
	p.m_MarkerGroup2.left.Destroy();
	p.m_MarkerGroup2.top.Destroy();
	p.m_MarkerGroup2.right.Destroy();
	p.m_MarkerGroup2.bottom.Destroy();
	p.m_MarkerGroup2.center.Destroy();
	p.m_MarkerGroup2.back.Destroy();
		// 
	p.m_InsertWindow.Destroy();
}
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::IsCreated() const
{	return ::IsWindow(p.m_MarkerGroup1.left.m_hWnd)!=0;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::ShowMarkers(CRect const *rcScr)
{	assert( IsCreated() );
		// 
	p.m_ShownGroup = (p.m_ShownGroup==Private::MarkerGroup::TypeFirst ? Private::MarkerGroup::TypeSecond : Private::MarkerGroup::TypeFirst);   // switch marker group.
	Private::MarkerGroup *pn = p.GetWorkingMarkerGroup();
		// 
		// set enable states for markers.
	pn->left.Disable( !p.m_pAbilityManager->IsMarkerEnable(this,PositionLeft) );
	pn->top.Disable( !p.m_pAbilityManager->IsMarkerEnable(this,PositionTop) );
	pn->right.Disable( !p.m_pAbilityManager->IsMarkerEnable(this,PositionRight) );
	pn->bottom.Disable( !p.m_pAbilityManager->IsMarkerEnable(this,PositionBottom) );
	pn->center.Disable( !p.m_pAbilityManager->IsMarkerEnable(this,PositionCenter) );
		// 
		// calculate left-top point.
	pn->basePoint.x = (rcScr->left + rcScr->right - p.m_Layout.widthTotal) / 2;
	pn->basePoint.y = (rcScr->top + rcScr->bottom - p.m_Layout.heightTotal) / 2;
		// positioning and show new markers.
	pn->back.SetPos(pn->basePoint+p.m_Layout.back.pos);
	pn->center.SetPos(pn->basePoint+p.m_Layout.center.pos);   // !!! before left,top,right and bottom.
	pn->left.SetPos(pn->basePoint+p.m_Layout.left.pos);
	pn->top.SetPos(pn->basePoint+p.m_Layout.top.pos);
	pn->right.SetPos(pn->basePoint+p.m_Layout.right.pos);
	pn->bottom.SetPos(pn->basePoint+p.m_Layout.bottom.pos);
	pn->animation.Show();
		// hide old markers.
	pn = p.GetNotWorkingMarkerGroup();
	pn->animation.Hide();
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::HideMarkers()
{	assert( IsCreated() );
		// 
	p.GetWorkingMarkerGroup()->animation.Hide();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::ShowInsertionArea(CWnd *parentWnd, CRect const *rcScr, bool center, bool tabsOnTop, int tabHeight)
{	if( IsCreated() )
	{	p.m_InsertWindow.SetParams(center,tabsOnTop,tabHeight);
			// 
		HWND hWndAfter = HWND_TOPMOST;
		if( ::IsWindowVisible(p.GetWorkingMarkerGroup()->back.m_hWnd) )
			hWndAfter = p.GetWorkingMarkerGroup()->back.m_hWnd;
		else if( ::IsWindowVisible(p.GetWorkingMarkerGroup()->center.m_hWnd) )
			hWndAfter = p.GetWorkingMarkerGroup()->center.m_hWnd;
		p.m_InsertWindow.Show(hWndAfter,rcScr);
	}
	else
		if( GetDrawManager() ) 
		{	p.m_bCenter = center;
			p.m_bTabsOnTop = tabsOnTop;
			p.m_iTabHeight = tabHeight;
				// 
			CRect rc(rcScr);
			parentWnd->ScreenToClient(&rc);
			p.DrawInsertionFrame(parentWnd,&rc);
		}
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::HideInsertionArea(CWnd *parentWnd)
{	if( IsCreated() )
		p.m_InsertWindow.Hide();
	else
		if( GetDrawManager() ) 
			p.DrawInsertionFrame(parentWnd,nullptr);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Private::DrawInsertionFrame(CWnd *parentWnd, CRect const *rect)
{	static CRect rcOld(0,0,0,0);
		// 
	if(rect || !rcOld.IsRectNull()) 
	{	CDC *dc = parentWnd->GetDCEx(nullptr, DCX_CACHE | DCX_LOCKWINDOWUPDATE);
		if(dc)
		{	o.GetDrawManager()->DrawInsertionFrame(&o,dc,rect,(!rcOld.IsRectNull() ? &rcOld : nullptr),
				m_bCenter,m_bTabsOnTop,m_iTabHeight);
			parentWnd->ReleaseDC(dc);
		}
	}
		// 
	rcOld = (rect ? *rect : CRect(0,0,0,0));
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Position DockingMarkers::RelayMouseMove(CPoint pt)
{	assert( IsCreated() );
		// 
	Position marker = p.HitTest(pt);
		// 
	if(marker!=PositionNone)
		if( !p.m_pAbilityManager->IsMarkerEnable(this,marker) )
			marker = PositionNone;
		// 
	Private::MarkerGroup *pn = p.GetWorkingMarkerGroup();
		// 
	switch(marker)
	{	case PositionLeft: pn->animation.Highlight(&pn->left,true); break;
		case PositionTop: pn->animation.Highlight(&pn->top,true); break;
		case PositionRight: pn->animation.Highlight(&pn->right,true); break;
		case PositionBottom: pn->animation.Highlight(&pn->bottom,true); break;
		case PositionCenter: pn->animation.Highlight(&pn->center,true); break;
	}
		// 
	if(marker!=PositionLeft) 
		pn->animation.Highlight(&pn->left,false);
	if(marker!=PositionTop) 
		pn->animation.Highlight(&pn->top,false);
	if(marker!=PositionRight) 
		pn->animation.Highlight(&pn->right,false);
	if(marker!=PositionBottom) 
		pn->animation.Highlight(&pn->bottom,false);
	if(marker!=PositionCenter) 
		pn->animation.Highlight(&pn->center,false);
		// 
	return marker;
}
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Position DockingMarkers::Private::HitTest(CPoint pt)
{	MarkerGroup *pn = GetWorkingMarkerGroup();
		// 
	if( (m_Layout.left.area+pn->basePoint+m_Layout.left.pos).PtInRect(pt) ) 
		return PositionLeft;
	if( (m_Layout.top.area+pn->basePoint+m_Layout.top.pos).PtInRect(pt) ) 
		return PositionTop;
	if( (m_Layout.right.area+pn->basePoint+m_Layout.right.pos).PtInRect(pt) ) 
		return PositionRight;
	if( (m_Layout.bottom.area+pn->basePoint+m_Layout.bottom.pos).PtInRect(pt) ) 
		return PositionBottom;
	if( (m_Layout.center.area+pn->basePoint+m_Layout.center.pos).PtInRect(pt) ) 
		return PositionCenter;
		// 
	return PositionNone;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::SetAbilityManager(Ability *ptr)
{	p.m_pAbilityManager = (ptr ? ptr : &p);
}
// 
DockingMarkers::Ability *DockingMarkers::GetAbilityManager() const
{	return p.m_pAbilityManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::SetDrawManager(Draw *ptr)
{	p.m_InsertWindow.SetDrawManager(ptr);
}
// 
DockingMarkers::Draw *DockingMarkers::GetDrawManager() const
{	return p.m_InsertWindow.GetDrawManager();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Private::MarkerGroup *DockingMarkers::Private::GetWorkingMarkerGroup()
{	return (m_ShownGroup==MarkerGroup::TypeFirst ? &m_MarkerGroup1 : &m_MarkerGroup2);
}
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Private::MarkerGroup *DockingMarkers::Private::GetNotWorkingMarkerGroup()
{	return (m_ShownGroup==MarkerGroup::TypeFirst ? &m_MarkerGroup2 : &m_MarkerGroup1);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::Private::LoadImage(HMODULE moduleRes/*or null*/, UINT resID, bool pngImage, Gdiplus::Bitmap **bmp/*out*/)
{	assert(resID);
	assert(bmp);
		// 
	*bmp = nullptr;
		// 
	if(!moduleRes)
		moduleRes = AfxFindResourceHandle(MAKEINTRESOURCE(resID),(pngImage ? _T("PNG") : RT_BITMAP));
	if(moduleRes)
	{	if(!pngImage)   // bmp.
			*bmp = ::new (std::nothrow) Gdiplus::Bitmap(moduleRes,MAKEINTRESOURCEW(resID));
		else   // png.
		{	HRSRC rsrc = ::FindResource(moduleRes,MAKEINTRESOURCE(resID),_T("PNG"));
			if(rsrc)
			{	HGLOBAL rsrcMem = ::LoadResource(moduleRes,rsrc);
				if(rsrcMem)
				{	const void *rsrcBuffer = ::LockResource(rsrcMem);
					if(rsrcBuffer)
					{	const UINT rsrcSize = static_cast<UINT>( ::SizeofResource(moduleRes,rsrc) );
						HGLOBAL streamMem = ::GlobalAlloc(GMEM_MOVEABLE,rsrcSize);
						if(streamMem)
						{	void *streamBuffer = ::GlobalLock(streamMem);
							if(streamBuffer)
							{	memcpy(streamBuffer,rsrcBuffer,rsrcSize);
								::GlobalUnlock(streamBuffer);
									// 
								IStream *stream = nullptr;
								if(::CreateStreamOnHGlobal(streamMem,FALSE,&stream/*out*/)==S_OK)
								{	*bmp = ::new (std::nothrow) Gdiplus::Bitmap(stream,FALSE);
									stream->Release();
								}
							}
							::GlobalFree(streamMem);
						}
						::UnlockResource(rsrcMem);
					}
					::FreeResource(rsrcMem);
				}
			}
		}
	}
	if(*bmp && (*bmp)->GetLastStatus()!=Gdiplus::Ok)
	{	::delete *bmp;
		*bmp = nullptr;
		return false;
	}
	return (*bmp)!=nullptr;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkersDrawBase.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
COLORREF DockingMarkersDrawBase::GetInsertionWindowTranspColor(DockingMarkers * /*markers*/)
{	return RGB(255,0,255);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDrawBase::DrawInsertionWindow(DockingMarkers *markers, CDC *dc, CRect const *rect, bool center, bool tabsOnTop, int tabHeight)
{	CRect rc(rect);
	const int width = GetInsertionWindowBorderWidth(markers);
		// 
	if(!center || rect->Width()<75 || rect->Height()<2*tabHeight)
	{	rc.DeflateRect(width,width);
			// 
		dc->ExcludeClipRect(&rc);
		DrawInsertionWindowBorder(markers,dc,rect);
		dc->SelectClipRgn(nullptr,RGN_COPY);
		DrawInsertionWindowInside(markers,dc,&rc);
	}
	else
	{	POINT pts1[8], pts2[8];
		GetOuterOutline(markers,&rc,tabsOnTop,tabHeight,width,pts1);
		GetInnerOutline(markers,&rc,tabsOnTop,tabHeight,width,pts2);
			// 
		CRgn rgn1, rgn2;
		rgn1.CreatePolygonRgn(pts1,sizeof(pts1)/sizeof(POINT),WINDING);
		rgn2.CreatePolygonRgn(pts2,sizeof(pts2)/sizeof(POINT),WINDING);
			// 
		dc->SelectClipRgn(&rgn1);
		dc->SelectClipRgn(&rgn2,RGN_DIFF);
		DrawInsertionWindowBorder(markers,dc,&rc);
		dc->SelectClipRgn(&rgn2,RGN_COPY);
		DrawInsertionWindowInside(markers,dc,&rc);
		dc->SelectClipRgn(nullptr,RGN_COPY);
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDrawBase::DrawInsertionFrame(DockingMarkers *markers, CDC *dc, CRect const *rectNew, CRect const *rectOld, bool center, bool tabsOnTop, int tabHeight)
{	const int width = GetInsertionFrameWidth(markers);
	CRect rc(rectNew ? rectNew : rectOld);
		// 
	if(!center || rc.Width()<75 || rc.Height()<2*tabHeight)
	{	CRect rcClip(rc);
		rcClip.DeflateRect(width,width);
		dc->ExcludeClipRect(&rcClip);
		DrawInsertionFrame(markers,dc,&rc);
	}
	else
	{	POINT pts1[8], pts2[8];
		GetOuterOutline(markers,&rc,tabsOnTop,tabHeight,width,pts1);
		GetInnerOutline(markers,&rc,tabsOnTop,tabHeight,width,pts2);
			// 
		CRgn rgn1, rgn2;
		rgn1.CreatePolygonRgn(pts1,sizeof(pts1)/sizeof(POINT),WINDING);
		rgn2.CreatePolygonRgn(pts2,sizeof(pts2)/sizeof(POINT),WINDING);
			// 
		dc->SelectClipRgn(&rgn1);
		dc->SelectClipRgn(&rgn2,RGN_DIFF);
		DrawInsertionFrame(markers,dc,&rc);
	}
		// 
	dc->SelectClipRgn(nullptr,RGN_COPY);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDrawBase::DrawInsertionWindowInside(DockingMarkers *markers, CDC *dc, CRect const *rect)
{	FillSolidRect(dc,rect, GetInsertionWindowInsideColor(markers) );
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDrawBase::DrawInsertionWindowBorder(DockingMarkers *markers, CDC *dc, CRect const *rect)
{	FillSolidRect(dc,rect, GetInsertionWindowBorderColor(markers) );
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDrawBase::DrawInsertionFrame(DockingMarkers * /*markers*/, CDC *dc, CRect const *rect)
{	CBrush *pOldBrush = dc->SelectObject( CDC::GetHalftoneBrush() );
	dc->PatBlt(rect->left,rect->top,rect->Width(),rect->Height(),PATINVERT);
	dc->SelectObject(pOldBrush);
}
/////////////////////////////////////////////////////////////////////////////
// 
int DockingMarkersDrawBase::GetInsertionWindowBorderWidth(DockingMarkers * /*markers*/)
{	return 3;
}
/////////////////////////////////////////////////////////////////////////////
// 
int DockingMarkersDrawBase::GetInsertionFrameWidth(DockingMarkers * /*markers*/)
{	return 4;
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF DockingMarkersDrawBase::GetInsertionWindowInsideColor(DockingMarkers * /*markers*/)
{	return ::GetSysColor(COLOR_HIGHLIGHT);
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF DockingMarkersDrawBase::GetInsertionWindowBorderColor(DockingMarkers * /*markers*/)
{	return ::GetSysColor(COLOR_ACTIVEBORDER);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDrawBase::GetOuterOutline(DockingMarkers * /*markers*/, CRect const *rect, bool top, int tabHeight, int width, POINT pts[8]/*out*/)
{	const int w = (width!=1 ? width/2 : 1);
		// 
	if(top)
	{	pts[0].x = rect->left; pts[0].y = rect->bottom;
		pts[1].x = rect->left; pts[1].y = rect->top+tabHeight-w;
		pts[2].x = rect->left+15; pts[2].y = rect->top+tabHeight-w;
		pts[3].x = rect->left+15; pts[3].y = rect->top;
		pts[4].x = rect->left+60; pts[4].y = rect->top;
		pts[5].x = rect->left+60; pts[5].y = rect->top+tabHeight-w;
		pts[6].x = rect->right; pts[6].y = rect->top+tabHeight-w;
		pts[7].x = rect->right; pts[7].y = rect->bottom;
	}
	else
	{	pts[0].x = rect->left; pts[0].y = rect->top;
		pts[1].x = rect->left; pts[1].y = rect->bottom-tabHeight+w;
		pts[2].x = rect->left+15; pts[2].y = rect->bottom-tabHeight+w;
		pts[3].x = rect->left+15; pts[3].y = rect->bottom;
		pts[4].x = rect->left+60; pts[4].y = rect->bottom;
		pts[5].x = rect->left+60; pts[5].y = rect->bottom-tabHeight+w;
		pts[6].x = rect->right; pts[6].y = rect->bottom-tabHeight+w;
		pts[7].x = rect->right; pts[7].y = rect->top;
	}
}
// 
void DockingMarkersDrawBase::GetInnerOutline(DockingMarkers * /*markers*/, CRect const *rect, bool top, int tabHeight, int width, POINT pts[8]/*out*/)
{	const int w = width / 2;
		// 
	if(top)
	{	pts[0].x = rect->left+width; pts[0].y = rect->bottom-width;
		pts[1].x = rect->left+width; pts[1].y = rect->top+tabHeight+w;
		pts[2].x = rect->left+15+width; pts[2].y = rect->top+tabHeight+w;
		pts[3].x = rect->left+15+width; pts[3].y = rect->top+width;
		pts[4].x = rect->left+60-width; pts[4].y = rect->top+width;
		pts[5].x = rect->left+60-width; pts[5].y = rect->top+tabHeight+w;
		pts[6].x = rect->right-width; pts[6].y = rect->top+tabHeight+w;
		pts[7].x = rect->right-width; pts[7].y = rect->bottom-width;
	}
	else
	{	pts[0].x = rect->left+width; pts[0].y = rect->top+width;
		pts[1].x = rect->left+width; pts[1].y = rect->bottom-tabHeight-w;
		pts[2].x = rect->left+15+width; pts[2].y = rect->bottom-tabHeight-w;
		pts[3].x = rect->left+15+width; pts[3].y = rect->bottom-width;
		pts[4].x = rect->left+60-width; pts[4].y = rect->bottom-width;
		pts[5].x = rect->left+60-width; pts[5].y = rect->bottom-tabHeight-w;
		pts[6].x = rect->right-width; pts[6].y = rect->bottom-tabHeight-w;
		pts[7].x = rect->right-width; pts[7].y = rect->top+width;
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDrawBase::FillSolidRect(CDC *dc, CRect const *rc, COLORREF color) const
{	HBRUSH hBrush = ::CreateSolidBrush(color);
	::FillRect(dc->m_hDC,rc,hBrush);
	::DeleteObject(hBrush);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// InsertionAreaDraw_custom1.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int DockingMarkersDraw_custom1::GetInsertionWindowBorderWidth(DockingMarkers * /*markers*/)
{	return 1;
}
/////////////////////////////////////////////////////////////////////////////
// 
int DockingMarkersDraw_custom1::GetInsertionFrameWidth(DockingMarkers * /*markers*/)
{	return 2;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDraw_custom1::DrawInsertionWindowInside(DockingMarkers * /*markers*/, CDC *dc, CRect const *rect)
{	CBrush brush;
	brush.CreateHatchBrush(HS_BDIAGONAL, RGB(110,110,110) );
	dc->SetBkColor( RGB(255,128,0) );
	dc->FillRect(rect,&brush);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDraw_custom1::DrawInsertionFrame(DockingMarkers * /*markers*/, CDC *dc, CRect const *rect)
{	CBrush br(HS_BDIAGONAL,::GetSysColor(COLOR_WINDOWTEXT));
		// 
	CBrush *pOldBrush = dc->SelectObject(&br);
	dc->PatBlt(rect->left,rect->top,rect->Width(),rect->Height(),PATINVERT);
	dc->SelectObject(pOldBrush);
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF DockingMarkersDraw_custom1::GetInsertionWindowBorderColor(DockingMarkers * /*markers*/)
{	return RGB(60,60,60);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkersDraw_VS2010.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int DockingMarkersDraw_VS2010::GetInsertionWindowBorderWidth(DockingMarkers * /*markers*/)
{	return 5;
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF DockingMarkersDraw_VS2010::GetInsertionWindowInsideColor(DockingMarkers * /*markers*/)
{	return MixingColors( ::GetSysColor(COLOR_HIGHLIGHT), RGB(255,255,255), 80);
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF DockingMarkersDraw_VS2010::MixingColors(COLORREF src, COLORREF dst, int percent) const
{	const int ipercent = 100 - percent;
	return RGB(
		(GetRValue(src) * percent + GetRValue(dst) * ipercent) / 100,
		(GetGValue(src) * percent + GetGValue(dst) * ipercent) / 100,
		(GetBValue(src) * percent + GetBValue(dst) * ipercent) / 100);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDraw_VS2010::GetOuterOutline(DockingMarkers * /*markers*/, CRect const *rect, bool top, int tabHeight, int /*width*/, POINT pts[8]/*out*/)
{	if(top)
	{	pts[0].x = rect->left; pts[0].y = rect->bottom;
		pts[1].x = rect->left; pts[1].y = rect->top;
		pts[2].x = rect->left; pts[2].y = rect->top;   // repeat.
		pts[3].x = rect->left; pts[3].y = rect->top;   // repeat.
		pts[4].x = rect->left+60; pts[4].y = rect->top;
		pts[5].x = rect->left+60; pts[5].y = rect->top+tabHeight;
		pts[6].x = rect->right; pts[6].y = rect->top+tabHeight;
		pts[7].x = rect->right; pts[7].y = rect->bottom;
	}
	else
	{	pts[0].x = rect->left; pts[0].y = rect->top;
		pts[1].x = rect->left; pts[1].y = rect->bottom;
		pts[2].x = rect->left; pts[2].y = rect->bottom;   // repeat.
		pts[3].x = rect->left; pts[3].y = rect->bottom;   // repeat.
		pts[4].x = rect->left+60; pts[4].y = rect->bottom;
		pts[5].x = rect->left+60; pts[5].y = rect->bottom-tabHeight;
		pts[6].x = rect->right; pts[6].y = rect->bottom-tabHeight;
		pts[7].x = rect->right; pts[7].y = rect->top;
	}
}
// 
void DockingMarkersDraw_VS2010::GetInnerOutline(DockingMarkers * /*markers*/, CRect const *rect, bool top, int tabHeight, int width, POINT pts[8]/*out*/)
{	if(top)
	{	pts[0].x = rect->left+width; pts[0].y = rect->bottom-width;
		pts[1].x = rect->left+width; pts[1].y = rect->top+width;
		pts[2].x = rect->left+width; pts[2].y = rect->top+width;   // repeat.
		pts[3].x = rect->left+width; pts[3].y = rect->top+width;   // repeat.
		pts[4].x = rect->left+60-width; pts[4].y = rect->top+width;
		pts[5].x = rect->left+60-width; pts[5].y = rect->top+tabHeight+width;
		pts[6].x = rect->right-width; pts[6].y = rect->top+tabHeight+width;
		pts[7].x = rect->right-width; pts[7].y = rect->bottom-width;
	}
	else
	{	pts[0].x = rect->left+width; pts[0].y = rect->top+width;
		pts[1].x = rect->left+width; pts[1].y = rect->bottom-width;
		pts[2].x = rect->left+width; pts[2].y = rect->bottom-width;   // repeat.
		pts[3].x = rect->left+width; pts[3].y = rect->bottom-width;   // repeat.
		pts[4].x = rect->left+60-width; pts[4].y = rect->bottom-width;
		pts[5].x = rect->left+60-width; pts[5].y = rect->bottom-tabHeight-width;
		pts[6].x = rect->right-width; pts[6].y = rect->bottom-tabHeight-width;
		pts[7].x = rect->right-width; pts[7].y = rect->top+width;
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////










