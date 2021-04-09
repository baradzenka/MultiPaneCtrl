//==========================================================
// Author: Baradzenka Aleh (baradzenka@gmail.com)
//==========================================================
// 
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MultiPaneCtrl.h"
#include <cassert>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <algorithm>
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4355)   // 'this' : used in base member initializer list.
#undef max
#undef min
#undef new
/////////////////////////////////////////////////////////////////////////////
// 
struct MultiPaneCtrl::Private :
	IRecalc,
	Ability,
	TabCtrl::Ability,
	TabCtrl::Notify,
	DockingMarkers::Ability
{
	template<typename T> struct tree
	{	tree() : m_NodeRoot(this)
		{	m_NodeRoot.parent = nullptr;
		}
		~tree()
		{	DeleteAll();
		}

	protected:
		struct Node
		{	Node(tree<T> *c)
			{	container = c;
			}
			~Node()
			{	for(typename std::list<Node *>::iterator i; !nodes.empty(); )   // from bottom to top.
				{	i = --nodes.end();
					delete *i;                       // 1 !!!.
					container->allnodes.erase(*i);   // 2 !!!.
					nodes.erase(i);                  // 3 !!!.
				}
			}
			tree<T> *container;
			Node *parent;
			T data;
			typename std::list<Node *>::iterator iterator;   // iterator of this node in the parent list.
			std::list<Node *> nodes;   // child nodes.
		} m_NodeRoot;
		typedef typename std::list<Node *>::iterator i_nodes;
		typedef typename std::list<Node *>::const_iterator ic_nodes;
			// 
		std::set<Node const *> allnodes;   // all nodes except root node.

	public:
		void operator=(tree<T> const &src)
		{	Copy(&src);
		}
		void Copy(tree<T> const *src)
		{	DeleteAll();
			try
			{	Copy(GetRoot(),src,src->GetRoot());
			}
			catch(std::bad_alloc &)
			{	DeleteAll();
				throw;
			}
		}

	private:
		void Copy(HPANE hDst, tree<T> const *treeSrc, HPANE hSrc)
		{	*GetData(hDst) = *treeSrc->GetData(hSrc);
				// 
			for(HPANE s=treeSrc->GetFirstChild(hSrc); s; s=treeSrc->GetNextChild(s))
			{	HPANE d = Add(hDst);
				Copy(d,treeSrc,s);
			}
		}

	public:
		HPANE Add(HPANE parent)
		{	assert(!parent || IsExist(parent));
				// 
			Node *pr = (parent ? HandleToNode(parent) : &m_NodeRoot);
				// 
			Node *n = new Node(this);
			try
			{	allnodes.insert(n);
				pr->nodes.push_back(n);
			}
			catch(std::bad_alloc &)
			{	if(allnodes.find(n)!=allnodes.end())
					allnodes.erase(n);
				delete n;
				throw;
			}
			n->parent = pr;
			n->iterator = --pr->nodes.end();
			return NodeToHandle(n);
		}
		HPANE Insert(HPANE before)
		{	assert( IsExist(before) );
			assert(before!=GetRoot());
				// 
			Node *b = HandleToNode(before);
			Node *pr = b->parent;
				// 
			Node *n = new Node(this);
			try
			{	allnodes.insert(n);
				n->iterator = pr->nodes.insert(b->iterator,n);
			}
			catch(std::bad_alloc &)
			{	if(allnodes.find(n)!=allnodes.end())
					allnodes.erase(n);
				delete n;
				throw;
			}
			n->parent = pr;
			return NodeToHandle(n);
		}
		void Remove(HPANE before, HPANE src)
		{	assert(before!=src);
			assert(IsExist(before) && IsExist(src));
			assert(GetParent(before)==GetParent(src));
				// 
			Node *pr = HandleToNode(before)->parent;
			std::list<Node *> tmp;
			tmp.splice(tmp.end(),pr->nodes, HandleToNode(src)->iterator );
				// 
			i_nodes i_before = HandleToNode(before)->iterator;
			pr->nodes.splice(i_before,tmp);
				// 
			HandleToNode(src)->iterator = --i_before;
		}
		void Delete(HPANE node)
		{	assert( IsExist(node) );
			assert(node!=GetRoot());
				// 
			Node *n = HandleToNode(node);
			Node *pr = n->parent;
				// 
			i_nodes i = n->iterator;
			delete n;             // 1 !!!.
			allnodes.erase(n);    // 2 !!!.
			pr->nodes.erase(i);   // 3 !!!.
		}
		void DeleteAll()
		{	for(i_nodes i; !m_NodeRoot.nodes.empty(); )
			{	i = --m_NodeRoot.nodes.end();
				delete *i;                   // 1 !!!.
				allnodes.erase(*i);          // 2 !!!.
				m_NodeRoot.nodes.erase(i);   // 3 !!!.
			}
		}
			// 
			//////////////// 
		HPANE InsertIntermediate(HPANE parent)
		{	assert(!parent || IsExist(parent));
			assert(GetNumber(parent)>0);
				// 
			Node *pr = (parent ? HandleToNode(parent) : &m_NodeRoot);
				// 
			Node *n = new Node(this);
			try
			{	allnodes.insert(n);
				pr->nodes.push_back(n);
			}
			catch(std::bad_alloc &)
			{	if(allnodes.find(n)!=allnodes.end())
					allnodes.erase(n);
				delete n;
				throw;
			}
				// 
			std::list<Node *> tmp;
			tmp.splice(tmp.end(),pr->nodes,pr->nodes.begin(),--pr->nodes.end());
			n->nodes.splice(n->nodes.end(),tmp);
				// 
			n->parent = pr;
			n->iterator = --pr->nodes.end();
			for(i_nodes i=n->nodes.begin(), e=n->nodes.end(); i!=e; ++i)
			{	(*i)->parent = n;
				(*i)->iterator = i;
			}
			return NodeToHandle(n);
		}
		void DeleteIntermediate(HPANE node)
		{	assert( IsExist(node) );
			assert(GetNumber(node)>0);
			assert( GetParent(node) );
			assert(GetNumber(GetParent(node))==1);
				// 
			Node *n = HandleToNode(node);
			Node *pr = n->parent;
				// 
			pr->nodes.swap( n->nodes );
			for(i_nodes i=pr->nodes.begin(), e=pr->nodes.end(); i!=e; ++i)
			{	(*i)->parent = pr;
				(*i)->iterator = i;
			}
			n->nodes.clear();    // 1 !!! - exclude recursion in the ~Node.
			delete n;            // 2 !!!.
			allnodes.erase(n);   // 3 !!!.
		}
			// 
			//////////////// 
		HPANE GetRoot() const
		{	return NodeToHandle(&m_NodeRoot);
		}
		HPANE GetParent(HPANE node) const
		{	assert( IsExist(node) );
				// 
			return NodeToHandle( HandleToNode(node)->parent );
		}
		HPANE GetNode(HPANE parent, int idx) const
		{	assert(!parent || IsExist(parent));
			assert(idx>=0 && idx<GetNumber(parent));
				// 
			Node const *pr = (parent ? HandleToNode(parent) : &m_NodeRoot);
				// 
			ic_nodes i = pr->nodes.begin();
			for(; idx>0; --idx)
				++i;
			return NodeToHandle(*i);
		}
		T *GetData(HPANE node)
		{	assert(node==nullptr || IsExist(node));
				// 
			return &(node ? HandleToNode(node) : &m_NodeRoot)->data;
		}
		T const *GetData(HPANE node) const 
		{	assert(node==nullptr || IsExist(node));
				// 
			return &(node ? HandleToNode(node) : &m_NodeRoot)->data;
		}
		int GetIndex(HPANE node) const
		{	assert( IsExist(node) );
			assert(node!=GetRoot());
				// 
			Node const *n = HandleToNode(node);
			Node const *pr = n->parent;
				// 
			int idx = 0;
			for(ic_nodes i=pr->nodes.begin(), e=pr->nodes.end(); i!=e; ++i,++idx)
				if(*i==n)
					return idx;
			return -1;
		}
		int GetNumber(HPANE parent) const
		{	assert(!parent || IsExist(parent));
				// 
			Node const *pr = (parent ? HandleToNode(parent) : &m_NodeRoot);
			return static_cast<int>( pr->nodes.size() );
		}
		int GetNumber() const
		{	return static_cast<int>( allnodes.size() ) + 1/*root*/;
		}
		bool IsExist(HPANE node) const
		{	assert(node);
				// 
			if(node==GetRoot())
				return true;
			return (allnodes.find( HandleToNode(node) )!=allnodes.end());
		}
			// 
			////////////////
		HPANE GetFirstChild(HPANE parent) const
		{	assert(!parent || IsExist(parent));
				// 
			Node const *pr = (parent ? HandleToNode(parent) : &m_NodeRoot);
			return (pr->nodes.empty() ? nullptr : NodeToHandle(pr->nodes.front()));
		}
		HPANE GetPrevChild(HPANE node) const
		{	assert( IsExist(node) );
			assert(node!=GetRoot());
				// 
			Node const *n = HandleToNode(node);
			Node const *pr = n->parent;
				// 
			typename std::list<Node *>::iterator i = n->iterator;
			return (i==pr->nodes.begin() ? nullptr : NodeToHandle(*--i));
		}
		HPANE GetNextChild(HPANE node) const
		{	assert( IsExist(node) );
			assert(node!=GetRoot());
				// 
			Node const *n = HandleToNode(node);
			Node const *pr = n->parent;
				// 
			typename std::list<Node *>::iterator i = n->iterator;
			return (++i==pr->nodes.end() ? nullptr : NodeToHandle(*i));
		}
		HPANE GetLastChild(HPANE parent) const
		{	assert(!parent || IsExist(parent));
				// 
			Node const *pr = (parent ? HandleToNode(parent) : &m_NodeRoot);
			return (pr->nodes.empty() ? nullptr : NodeToHandle(*--pr->nodes.end()));
		}
			// 
			//////////////// 
		HPANE GetFirst() const
		{	return GetFirst( NodeToHandle(&m_NodeRoot) );
		}
		HPANE GetFirst(HPANE parent) const
		{	assert( IsExist(parent) );
				// 
			return parent;
		}
			// 
		HPANE GetPrev(HPANE node) const
		{	return GetPrev( NodeToHandle(&m_NodeRoot), node);
		}
		HPANE GetPrev(HPANE parent, HPANE node) const
		{	assert(IsExist(parent) && IsExist(node));
				// 
			if(node==parent)
				return nullptr;
				// 
			Node const *n = HandleToNode(node);
				// 
			std::list<Node *> const *list = &n->parent->nodes;
			ic_nodes i = n->iterator;
				// 
			if(i==list->begin())
				return NodeToHandle( (*i)->parent );
				// 
			for(; !(*--i)->nodes.empty(); i=(*i)->nodes.end());
				// 
			return NodeToHandle(*i);
		}
			// 
		HPANE GetNext(HPANE node) const
		{	return GetNext( NodeToHandle(&m_NodeRoot), node);
		}
		HPANE GetNext(HPANE parent, HPANE node) const
		{	assert(IsExist(parent) && IsExist(node));
				// 
			Node const *n = HandleToNode(node);
				// 
			if( !n->nodes.empty() )
				return NodeToHandle( n->nodes.front() );
			if(node==parent)
				return nullptr;
				// 
			std::list<Node *> *list = &n->parent->nodes;
			ic_nodes i = n->iterator;
				// 
			while(++i==list->end())
			{	Node *pr = (*--i)->parent;
				if(pr==HandleToNode(parent))
					return nullptr;
				list = &pr->parent->nodes;
				i = pr->iterator;
			}
				// 
			return NodeToHandle(*i);
		}
			// 
		HPANE GetLast() const
		{	return GetLast( NodeToHandle(&m_NodeRoot) );
		}
		HPANE GetLast(HPANE parent) const
		{	assert( IsExist(parent) );
				// 
			if( HandleToNode(parent)->nodes.empty() )
				return parent;
				// 
			ic_nodes i;
			for(i = HandleToNode(parent)->nodes.end(); !(*--i)->nodes.empty(); i=(*i)->nodes.end());
			return NodeToHandle(*i);
		}

	protected:
		HPANE NodeToHandle(Node const *node) const
		{	return reinterpret_cast<HPANE>(node);
		}
		Node const *HandleToNode(HPANE node) const
		{	return reinterpret_cast<Node const *>(node);
		}
		Node *HandleToNode(HPANE node)
		{	return const_cast<Node *>( reinterpret_cast<Node const *>(node) );
		}
	};
		//
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
	struct DeferWindow
	{	~DeferWindow()
		{	Process();
		}

	public:
		bool MoveShowWindow(HWND wnd, RECT const *rc, bool redraw)   // move and show window.
		{	assert( ::IsWindow(wnd) );
				// 
			return Add(wnd,rc,redraw,true);
		}
			// 
		void Process()
		{	const int count = static_cast<int>( windows.size() );
				// 
			if(count>0)
			{	HDWP dwp = ::BeginDeferWindowPos(count);
					// 
				if(dwp)
				{	for(i_windows i=windows.begin(), e=windows.end(); i!=e; ++i)
					{	Window &w = *i;
						const bool needShow = (w.show && !::IsWindowVisible(w.wnd));
							// 
						if(!HasWindowPos(w.wnd,&w.rc) || needShow)
							::DeferWindowPos(dwp,w.wnd, nullptr, w.rc.left,w.rc.top,w.rc.Width(),w.rc.Height(),
								SWP_NOZORDER | SWP_NOACTIVATE | (needShow ? SWP_SHOWWINDOW : 0) | (w.redraw ? 0 : SWP_NOREDRAW));
					}
						// 
					::EndDeferWindowPos(dwp);
				}
				else
					for(i_windows i=windows.begin(), e=windows.end(); i!=e; ++i)
					{	Window &w = *i;
						const bool needShow = (w.show && !::IsWindowVisible(w.wnd));
							// 
						if(!HasWindowPos(w.wnd,&w.rc) || needShow)
							::SetWindowPos(w.wnd, nullptr, w.rc.left,w.rc.top,w.rc.Width(),w.rc.Height(),
								SWP_NOZORDER | SWP_NOACTIVATE | (needShow ? SWP_SHOWWINDOW : 0) | (w.redraw ? 0 : SWP_NOREDRAW));
					}
					// 
				windows.clear();
			}
		}

	private:
		struct Window
		{	HWND wnd;
			CRect rc;
			bool redraw, show;
		};
		std::list<Window> windows;
		typedef std::list<Window>::iterator i_windows;

	private:
		bool Add(HWND wnd, RECT const *rc, bool redraw, bool show)
		{	try
			{	windows.push_back( Window() );
				Window &w = windows.back();
				w.wnd = wnd;
				w.rc = rc;
				w.redraw = redraw;
				w.show = show;
			}
			catch(std::bad_alloc &)
			{	return false;
			}
			return true;
		}
			// 
		bool HasWindowPos(HWND wnd, RECT const *rc) const   // return 'true' if window has 'rc' position.
		{	CRect rcNow;
			::GetWindowRect(wnd,&rcNow/*out*/);
			::MapWindowPoints(HWND_DESKTOP, ::GetParent(wnd), reinterpret_cast<POINT*>(&rcNow),2);   // screen to parent's client.
			return (rcNow==*rc)!=0;
		}
	};
		// 
	template<typename T> struct KeyboardHook
	{	void Add(T *obj, bool(T::*func)(UINT,UINT))
		{	Data *data = Lock();
			data->clients[obj] = func;
			Unlock(data);
		}
			// 
		void Delete(T *obj)
		{	Data *data = Lock();
			data->clients.erase(obj);
			Unlock(data);
		}

	private:
		struct Data : CRITICAL_SECTION
		{	Data()
			{	hook = ::SetWindowsHookEx(WH_KEYBOARD,static_cast<HOOKPROC>(HookProc),nullptr,::GetCurrentThreadId());
				::InitializeCriticalSection(this);
			}
			~Data()
			{	if(hook)
					::UnhookWindowsHookEx(hook);
				::DeleteCriticalSection(this);
			}
				// 
			HHOOK hook;
			std::map<T *,bool(T::*)(UINT,UINT)> clients;
		};
		static Data *GetData() { static Data data; return &data; }
			// 
		static Data *Lock() { Data *data=GetData(); ::EnterCriticalSection(data); return data; }
		static void Unlock(Data *data) { ::LeaveCriticalSection(data); }

	private:
		static LRESULT __stdcall HookProc(int code, WPARAM wParam, LPARAM lParam)
		{	Data *data = GetData();
				// 
			if(code==HC_ACTION &&
				!(lParam & 0x80000000))   // key is down.
			{
				Lock();
					// 
				typename std::map<T *,bool(T::*)(UINT,UINT)>::const_iterator i, n;
				for(i=data->clients.begin(); i!=data->clients.end(); )
				{	n = i++;
					if( !(n->first->*n->second)(static_cast<UINT>(wParam),static_cast<UINT>(lParam)) )
					{	Unlock(data);
						return 1;   // to prevent calling target window procedure (any nonzero value acceptable).
					}
				}
					// 
				Unlock(data);
			}
			return ::CallNextHookEx(data->hook,code,wParam,lParam);
		}
	};
		// 
	template<typename T> struct ActivityHook
	{	typedef std::pair<T *, void(T::*)(bool,HWND)> target_t;
			// 
		void Add(HWND wnd, T *obj, void(T::*func)(bool,HWND))
		{	Data *data = Lock();
			data->clients[wnd] = target_t(obj,func);
			Unlock(data);
		}
			// 
		void Delete(HWND wnd)
		{	Data *data = Lock();
			data->clients.erase(wnd);
			Unlock(data);
		}

	private:
		struct Data : CRITICAL_SECTION
		{	Data()
			{	hook = ::SetWindowsHookEx(WH_CALLWNDPROC,static_cast<HOOKPROC>(HookProc),nullptr,::GetCurrentThreadId());
				::InitializeCriticalSection(this);
			}
			~Data()
			{	if(hook)
					::UnhookWindowsHookEx(hook);
				::DeleteCriticalSection(this);
			}
				// 
			HHOOK hook;
			std::map<HWND,target_t> clients;
		};
		static Data *GetData() { static Data data; return &data; }
			// 
		static Data *Lock() { Data *data=GetData(); ::EnterCriticalSection(data); return data; }
		static void Unlock(Data *data) { ::LeaveCriticalSection(data); }

	private:
		static LRESULT __stdcall HookProc(int code, WPARAM wParam, LPARAM lParam)
		{	Data *data = GetData();
				// 
			if(code==HC_ACTION)
			{	CWPSTRUCT const *info = reinterpret_cast<CWPSTRUCT const *>(lParam);
					// 
				if(info->message==WM_SETFOCUS || info->message==WM_KILLFOCUS)
				{	Lock();
					CallClient(data,info->hwnd,info->message==WM_SETFOCUS);
					Unlock(data);
				}
			}
			return ::CallNextHookEx(data->hook,code,wParam,lParam);
		}
			// 
		static void CallClient(Data *data, HWND hwnd, bool value)
		{	typename std::map<HWND,target_t>::const_iterator i,n,e;
			for(i=data->clients.begin(), e=data->clients.end(); i!=e; )
			{	n = i++;
					// 
				for(HWND h=hwnd; h; h=::GetParent(h))
					if(h==n->first)
					{	(n->second.first->*n->second.second)(value,hwnd);
						break;
					}
			}
		}
	};
		// 
	struct TabImpl : MultiPaneCtrl::ITab
	{	HWND wnd;
		CString text;
		int image;
		CString tooltip;
		bool disable;
		__int64 data;
			// 
		HWND GetWnd() const override { return wnd; }
		CString GetText() const override { return text; }
		int GetImage() const override { return image; }
		CString GetToolTipText() const override { return tooltip; }
		bool IsDisable() const override { return disable; }
		__int64 GetData() const override { return data; }
	};
		// 
	struct TabsData
	{	std::vector<TabImpl *> tabs;
		std::map<int/*id*/,TabImpl> id_to_tab;
			// 
		static TabImpl const *GetTabWithCtrlID(Tabs const *tabs, int id)
		{	if(tabs->Owner)
			{	TabsData const *data = static_cast<TabsData const *>(tabs->Owner);
				std::map<int,TabImpl>::const_iterator ic = data->id_to_tab.find(id);
				return (ic!=data->id_to_tab.end() ? &ic->second : nullptr);
			}
			return nullptr;
		}
	};

public:
	Private(MultiPaneCtrl &owner);
	~Private();

private:
	MultiPaneCtrl &o;

private: // IMultiPaneCtrlRecalc.
	int GetBorderWidth(MultiPaneCtrl const *ctrl, IRecalc *base) override;
	CSize GetSplitterSize(MultiPaneCtrl const *ctrl, IRecalc *base) override;

private: // TabCtrl::Ability.
	bool CanShowButtonClose(TabCtrl const *ctrl) override;
	bool CanShowButtonMenu(TabCtrl const *ctrl) override;
	bool CanShowButtonScroll(TabCtrl const *ctrl) override;

private: // TabCtrl::Notify.
	void OnTabPreCreate(TabCtrl const *ctrl, HWND hWnd, TCHAR const *text, int image) override;
	void OnTabPostCreate(TabCtrl *ctrl, TabCtrl::HTAB hTab) override;
	void OnTabPreDestroy(TabCtrl const *ctrl, TabCtrl::HTAB hTab) override;
		// 
	void OnButtonCloseClicked(TabCtrl *ctrl, CRect const *rect, CPoint ptScr) override;
	void OnButtonMenuClicked(TabCtrl *ctrl, CRect const *rect, CPoint ptScr) override;
	void OnTabSelected(TabCtrl *ctrl, TabCtrl::HTAB hTab) override;
	void OnLButtonDown(TabCtrl const *ctrl, TabCtrl::HTAB hTab, CPoint ptScr) override;
	void OnLButtonDblClk(TabCtrl *ctrl, TabCtrl::HTAB hTab, CPoint ptScr) override;
	void OnRButtonDown(TabCtrl *ctrl, TabCtrl::HTAB hTab, CPoint ptScr) override;
	void OnRButtonUp(TabCtrl *ctrl, TabCtrl::HTAB hTab, CPoint ptScr) override;
		// 
	void OnStartDrag(TabCtrl const *ctrl, TabCtrl::HTAB hTab, CPoint ptScr) override;
	void OnDrag(TabCtrl *ctrl, TabCtrl::HTAB hTab, CPoint ptScr, bool outside) override;
	void OnFinishDrag(TabCtrl *ctrl, TabCtrl::HTAB hTab, bool cancel) override;

public: // DockingMarkersAbility.
	bool IsMarkerEnable(DockingMarkers *pObj, DockingMarkers::Position dockingSide) override;

public:
	void OnActive(bool active, HWND wnd);   // callback from ActivityHook.
	bool OnKeyDown(UINT keyCode, UINT msgFlag);   // callback from KeyboardHook.

public:
	Draw *m_pDrawManager;
	IRecalc *m_pRecalcManager;
	MultiPaneCtrl::Ability *m_pAbilityManager;
	MultiPaneCtrl::Notify *m_pNotifyManager;
	ITabCtrlStyle *m_pTabStyle;
		// 
	DockingMarkers m_DockMarkMngr;
		// 
	struct Image
	{	Gdiplus::Bitmap *bmp;
		CSize size;
	} m_ImageSys, m_ImageNormal,m_ImageDisable;
	COLORREF m_clrImageSysTransp, m_clrImageTransp;
	HCURSOR m_hCurTab, m_hCurSplitterHorz,m_hCurSplitterVert, m_hCurDragEnable,m_hCurDragDisable;
	HCURSOR *m_hpCurTabRef, *m_hpCurSplitterHorzRef,*m_hpCurSplitterVertRef, 
		*m_hpCurDragEnableRef,*m_hpCurDragDisableRef;
	CFont m_Font,*m_pFontRef, m_FontSelect,*m_pFontSelectRef;
		// 
	bool m_bShowBorder;
	bool m_bDragTabEnable;
	bool m_bDropOptimiz;
	CSize m_szMinSizeForDrop;
	CSize m_szMinSize;
		// 
	TabCtrl m_TabState;
	SplitterDragging m_SplitterDragMode;

public:
	struct Pane
	{	Pane();
		~Pane();
			// 
		TabCtrl *tab;   // null if there are child panes, otherwise pane is empty or has tabs.
		bool horz;   // orientation of line.
		__int64 data;   // data of pane.
			// 
		MultiPaneCtrl *owner;
		HPANE pane;
			// 
		struct State
		{	CRect rc, rcSplitter;
			double factor;
		} real, store;
			// 
		bool activeSplitter;
	};
	tree<Pane> m_Tree;
		// 
	HPANE m_hSplitDragPane;
	CPoint m_ptSplitDragStart;
		// 
	ActivityHook<Private> m_ActivityHook;
	KeyboardHook<Private> m_KeyboardHook;
		// 
	bool m_bActive;
	HWND m_hLastActiveWnd;
		// 
	TabCtrl::HTAB m_hDraggingTab;
	HPANE m_hMarkedPane;
	DockingMarkers::Position m_SelectedMarker;
	bool m_bDragFinishing;

public:
	void PreCreateTabCtrl(TabCtrl *tab);
	bool CreatePane(HPANE pane, bool createTab);
	void DestroyPane(HPANE pane);
	void Recalc(bool redraw);
	void Recalc(DeferWindow *dw, HPANE pane, CRect rc, bool redraw);
	HPANE HitTestSplitter(CPoint const *pt);
	void StartSplitterDragging(CPoint point);
	void StopSplitterDragging(bool reset);
	void SetDraggingWidth(HPANE pane, int offset, CRect *rcdrag/*out*/);
	void SetDraggingHeight(HPANE pane, int offset, CRect *rcdrag/*out*/);
	void DrawSplitterDragRect(CRect const *rect, bool horz);
	bool IsSplitterHorizontal(HPANE pane) const;
	bool CanSplitterDrag(HPANE pane);
	void HalveWidthPanes(HPANE prev, HPANE next);
	void HalveHeightPanes(HPANE prev, HPANE next);
	void SaveAllPanesState();
	void RestoreAllPanesState();
	void UpdateAdjacentPanes(HPANE pane1, HPANE pane2);
	DockingMarkers::Position RelayMouseMove(TabCtrl *pTabDst, CPoint ptScr);
	void DeleteSrcTab(HPANE paneSrc, TabCtrl *pTabSrc, TabCtrl::HTAB hTabSrc, HPANE paneMarked, DockingMarkers::Position dockingSide);
	void DeleteOptimizDown(HPANE paneSrc, bool optimize, Space space);
	void SaveStateInner(CArchive *ar) const;
	void SaveStateInner(CArchive *ar, HPANE pane) const;
	bool LoadStateInner(CArchive *ar, Tabs const *tabs, bool ignoreLack);
	bool LoadStateInner(CArchive *ar, HPANE parent, Tabs const *tabs, bool ignoreLack, std::list<HPANE> *panes/*out*/);
	bool LoadImage(HMODULE moduleRes/*or null*/, UINT resID, bool pngImage, Gdiplus::Bitmap **bmp/*out*/) const;
	bool IsDescendantWindow(HWND wndDst, HWND wndSrc) const;
	bool PtInTriangle(CPoint const &pt, CPoint const &v1, CPoint const &v2, CPoint const &v3) const;
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// MultiPaneCtrl.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(MultiPaneCtrl,CWnd)
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(MultiPaneCtrl, CWnd)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CAPTURECHANGED()
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::MultiPaneCtrl() :
	p( *new Private(*this) )
{
}
// 
MultiPaneCtrl::~MultiPaneCtrl()
{	delete &p;
}
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::Private::Private(MultiPaneCtrl &owner) : o(owner)
{	m_pDrawManager = nullptr;
	m_pRecalcManager = this;
	m_pAbilityManager = this;
	m_pNotifyManager = nullptr;
	m_pTabStyle = nullptr;
		// 
	m_DockMarkMngr.SetAbilityManager(this);
		// 
	m_ImageSys.bmp = m_ImageNormal.bmp = m_ImageDisable.bmp = nullptr;
	m_ImageSys.size.SetSize(0,0);
	m_ImageNormal.size = m_ImageDisable.size = m_ImageSys.size;
	m_hCurTab = m_hCurSplitterHorz = m_hCurSplitterVert = m_hCurDragEnable = m_hCurDragDisable = nullptr;
	m_hpCurTabRef = m_hpCurSplitterHorzRef = m_hpCurSplitterVertRef = 
		m_hpCurDragEnableRef = m_hpCurDragDisableRef = nullptr;
	m_pFontRef = m_pFontSelectRef = nullptr;
		// 
	m_bShowBorder = false;
	m_bDragTabEnable = false;
	m_bDropOptimiz = true;
	m_szMinSizeForDrop.cx = m_szMinSizeForDrop.cy = 70;
	m_szMinSize.cx = m_szMinSize.cy = 10;
		// 
	m_SplitterDragMode = SplitterDraggingStatic;
}
// 
MultiPaneCtrl::Private::~Private()
{	o.DestroyWindow();
		// 
	::delete m_ImageSys.bmp;
	::delete m_ImageNormal.bmp;
	::delete m_ImageDisable.bmp;
		// 
	if(m_hCurTab)
		::DestroyCursor(m_hCurTab);
	if(m_hCurSplitterHorz)
		::DestroyCursor(m_hCurSplitterHorz);
	if(m_hCurSplitterVert)
		::DestroyCursor(m_hCurSplitterVert);
	if(m_hCurDragEnable)
		::DestroyCursor(m_hCurDragEnable);
	if(m_hCurDragDisable)
		::DestroyCursor(m_hCurDragDisable);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
BOOL MultiPaneCtrl::Create(LPCTSTR /*lpszClassName*/, LPCTSTR /*lpszWindowName*/, DWORD style, const RECT &rect, CWnd *parentWnd, UINT nID, CCreateContext * /*context*/)
{	return Create(parentWnd,style,rect,nID);
}
// 
bool MultiPaneCtrl::Create(CWnd *parent, DWORD style, RECT const &rect, UINT id)
{	p.m_hSplitDragPane = nullptr;
		// 
	p.m_bActive = false;
	p.m_hLastActiveWnd = nullptr;
		// 
	p.m_hMarkedPane = nullptr;
	p.m_SelectedMarker = DockingMarkers::PositionNone;
	p.m_bDragFinishing = false;
		// 
	const CString className = AfxRegisterWndClass(CS_DBLCLKS,::LoadCursor(nullptr,IDC_ARROW),nullptr,nullptr);
	if( !CWnd::Create(className,_T(""),style | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,rect,parent,id) )
		return false;
		// 
	CFont *font = CFont::FromHandle( static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT)) );
	if( !GetFontNormal() )
		SetFontNormal(font);
	if( !GetFontSelect() )
		SetFontSelect(font);
		// 
	if( !p.CreatePane(p.m_Tree.GetRoot(),true) )
		return false;
		// 
	if( IsWatchCtrlActivity() )
		p.m_ActivityHook.Add(m_hWnd, &p,&Private::OnActive);
		// 
	return true;
}
// 
void MultiPaneCtrl::OnDestroy()
{	DeleteAllPanes();
		// 
	Private::Pane *rootPane = p.m_Tree.GetData(nullptr);
	if(rootPane->tab)
	{	delete rootPane->tab;   // delete TabCtrl of root pane.
		rootPane->tab = nullptr;   // !!! not delete (necessary for the DestroyPane(...)).
	}
		// 
	if( IsWatchCtrlActivity() )
		p.m_ActivityHook.Delete(m_hWnd);
		// 
	CWnd::OnDestroy();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::OnActive(bool active, HWND wnd)
{	m_bActive = active;
		// 
	if(!active)   // kill focus.
		m_hLastActiveWnd = wnd;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnSetFocus(CWnd *pOldWnd)
{	CWnd::OnSetFocus(pOldWnd);
		// 
	if(p.m_hLastActiveWnd && ::IsWindow(p.m_hLastActiveWnd) && 
		p.IsDescendantWindow(m_hWnd,p.m_hLastActiveWnd))
		::SetFocus(p.m_hLastActiveWnd);
	else
	{	HPANE pane = GetFirstPaneWithTabs();
		if(pane)
			GetTabCtrl(pane)->SetFocus();
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::Private::CreatePane(HPANE pane, bool createTab)
{	Pane *pn = m_Tree.GetData(pane);
		// 
	pn->tab = nullptr;
		// 
	if(createTab)
	{	try
		{	pn->tab = new TabCtrl;
			PreCreateTabCtrl(pn->tab);
			if( !pn->tab->Create(&o,WS_CHILD,CRect(0,0,0,0),100) )
				throw std::bad_alloc();
		}
		catch(std::bad_alloc &)
		{	if(pn->tab)
				delete pn->tab;
			return false;
		}
	}
	pn->horz = false;
	pn->data = 0;
	pn->owner = &o;
	pn->pane = pane;
	pn->real.factor = 1.0;
	pn->activeSplitter = true;
		// 
	if(m_pNotifyManager)
		m_pNotifyManager->OnPanePostCreate(&o,pane);
		// 
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::DestroyPane(HPANE pane)
{	if(m_pNotifyManager)
		m_pNotifyManager->OnPanePreDestroy(&o,pane);
		// 
	if( m_Tree.IsExist(pane) )
	{	HPANE parent = m_Tree.GetParent(pane);
			// 
		if(parent && m_Tree.GetNumber(parent)==1 && m_Tree.GetNumber(pane)==0)
			m_Tree.GetData(parent)->tab = m_Tree.GetData(pane)->tab;
		else
		{	Pane *pn = m_Tree.GetData(pane);
				// 
			if(pn->tab)
			{	for(HWND h; (h=::GetWindow(pn->tab->m_hWnd,GW_CHILD))!=nullptr; )   // for all child windows of TabCtrl.
				{	::ShowWindow(h,SW_HIDE);
					::SetParent(h,o.m_hWnd);
				}
					// 
				delete pn->tab;
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::Private::Pane::Pane()
{	tab = nullptr;
	owner = nullptr;
}
// 
MultiPaneCtrl::Private::Pane::~Pane()
{	if(owner)
		owner->p.DestroyPane(pane);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnSize(UINT nType, int cx, int cy)
{	CWnd::OnSize(nType, cx, cy);
		// 
	p.Recalc(true);
	::RedrawWindow(m_hWnd,nullptr,nullptr,RDW_INVALIDATE | RDW_UPDATENOW);   // update window.
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Update()
{	assert( GetSafeHwnd() );
		// 
	p.Recalc(true);
	Invalidate(FALSE);
	UpdateAllTabs();
}
// 
void MultiPaneCtrl::Update(HPANE pane)
{	assert( GetSafeHwnd() );
	assert(pane==nullptr || IsPaneExist(pane));
		// 
	if(GetNumberPanes(pane)>0)
		for(HPANE h=GetFirstChildPane(pane); h; h=GetNextChildPane(h))
			Update(h);
	else
		GetTabCtrl(pane)->Update();
		// 
	CRect rc = GetPaneRect(pane);
	InvalidateRect(&rc,FALSE);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::UpdateAllTabs()
{	assert( GetSafeHwnd() );
		// 
	for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
		if(GetNumberPanes(h)==0)
			GetTabCtrl(h)->Update();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::Recalc(bool redraw)
{	CRect rc;
	o.GetClientRect(&rc);
		// 
	if(m_bShowBorder)
	{	const int width = o.GetBorderWidth();
		rc.DeflateRect(width,width);
	}
		// 
	DeferWindow dw;
	Recalc(&dw,nullptr,rc,redraw);
	m_Tree.GetData(nullptr)->real.rcSplitter.SetRectEmpty();
}
// 
void MultiPaneCtrl::Private::Recalc(DeferWindow *dw, HPANE pane, CRect rc, bool redraw)
{	Pane *pn = m_Tree.GetData(pane);
		// 
	if(pn->owner)
	{	const int count = m_Tree.GetNumber(pane);
		pn->real.rc = rc;
			// 
		if(count==0)   // pane has tabs.
			dw->MoveShowWindow(pn->tab->m_hWnd,&rc,redraw);
		else   // pane has child panes.
		{	CRect rect = rc;
			const CSize splitterSize = o.GetSplitterSize();
			HPANE h, hLast=m_Tree.GetLastChild(pane);
			int correctedTail=0, totalOver=0;
			int cx,cy;
				// 
			if(pn->horz)   // horizontal line.
			{	int iMinSizeCX = m_szMinSize.cx;
					// 
				const int totalWidth = std::max<int>(0, rc.Width()-(count-1)*splitterSize.cx);
				if(totalWidth < count*iMinSizeCX)
					iMinSizeCX = std::max(0, totalWidth / count);
					// 
				int pos = rc.left;
					// 
				for(h=m_Tree.GetFirstChild(pane); h; h=m_Tree.GetNextChild(h))
				{	pn = m_Tree.GetData(h);
						// 
					if(h!=hLast)
						cx = static_cast<int>(static_cast<double>(totalWidth) * pn->real.factor + 0.5);
					else   // last column.
						cx = rc.right-pos;
						// 
					if(cx<=iMinSizeCX)
						correctedTail += iMinSizeCX-cx;
					else
						totalOver += cx-iMinSizeCX;
					pos += (cx + splitterSize.cx);
				}
					// 
				pos = rc.left;
					// 
				for(h=m_Tree.GetFirstChild(pane); h; h=m_Tree.GetNextChild(h))
				{	pn = m_Tree.GetData(h);
						// 
					if(h!=hLast)
						cx = static_cast<int>(static_cast<double>(totalWidth) * pn->real.factor + 0.5);
					else   // last column.
						cx = rc.right-pos;
						// 
					if(h!=hLast)   // except last column.
						if(cx>iMinSizeCX)
							cx -= static_cast<int>(static_cast<float>(correctedTail) * (static_cast<float>(cx - iMinSizeCX) / static_cast<float>(totalOver)) + 0.5f);
					cx = std::max(cx,iMinSizeCX);
						// 
					rect.left = pos;
					rect.right = pos + cx;
					Recalc(dw,h,rect,redraw);
					pos += (cx + splitterSize.cx);
						// 
					if(h!=hLast)
						pn->real.rcSplitter.SetRect(rect.right,rect.top,rect.right+splitterSize.cx,rect.bottom);
					else   // last pane in line.
						pn->real.rcSplitter.SetRectEmpty();
				}
			}
			else   // vertical line.
			{	int iMinSizeCY = m_szMinSize.cy;
					// 
				const int totalHeight = std::max<int>(0, rc.Height()-(count-1)*splitterSize.cy);
				if(totalHeight < count*iMinSizeCY)
					iMinSizeCY = std::max(0, totalHeight / count);
					// 
				int pos = rc.top;
					// 
				for(h=m_Tree.GetFirstChild(pane); h; h=m_Tree.GetNextChild(h))
				{	pn = m_Tree.GetData(h);
						// 
					if(h!=hLast)
						cy = static_cast<int>(static_cast<double>(totalHeight) * pn->real.factor + 0.5);
					else   // last column.
						cy = rc.bottom-pos;
						// 
					if(cy<=iMinSizeCY)
						correctedTail += iMinSizeCY-cy;
					else
						totalOver += cy-iMinSizeCY;
					pos += (cy + splitterSize.cy);
				}
					// 
				pos = rc.top;
					// 
				for(h=m_Tree.GetFirstChild(pane); h; h=m_Tree.GetNextChild(h))
				{	pn = m_Tree.GetData(h);
						// 
					if(h!=hLast)
						cy = static_cast<int>(static_cast<double>(totalHeight) * pn->real.factor + 0.5);
					else   // last column.
						cy = rc.bottom-pos;
						// 
					if(h!=hLast)   // except last column.
						if(cy>iMinSizeCY)
							cy -= static_cast<int>(static_cast<float>(correctedTail) * (static_cast<float>(cy - iMinSizeCY) / static_cast<float>(totalOver)) + 0.5f);
					cy = std::max(cy,iMinSizeCY);
						// 
					rect.top = pos;
					rect.bottom = pos + cy;
					Recalc(dw,h,rect,redraw);
					pos += (cy + splitterSize.cy);
						// 
					if(h!=hLast)
						pn->real.rcSplitter.SetRect(rect.left,rect.bottom,rect.right,rect.bottom+splitterSize.cy);
					else   // last pane in line.
						pn->real.rcSplitter.SetRectEmpty();
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnPaint()
{	if(!p.m_pDrawManager)
	{	CPaintDC dc(this);
		return;
	}
		// 
	Private::VirtualWindow virtwnd(this);
	if( !virtwnd.GetSafeHdc() )
	{	CPaintDC dc(this);
		return;
	}
		// 
	p.m_pDrawManager->DrawBegin(this,&virtwnd);
		// 
	for(HPANE h=p.m_Tree.GetFirst(); h; h=p.m_Tree.GetNext(h))
	{	Private::Pane *pn = p.m_Tree.GetData(h);
			// 
		if( !pn->real.rcSplitter.IsRectEmpty() )
		{	const bool horz = !p.m_Tree.GetData( p.m_Tree.GetParent(h) )->horz;   // orientation of splitter.
			p.m_pDrawManager->DrawSplitter(this,&virtwnd,horz,&pn->real.rcSplitter);
		}
	}
		// 
	if(p.m_bShowBorder)
		if(GetBorderWidth()>0)
		{	CRect rc;
			GetClientRect(&rc);
			p.m_pDrawManager->DrawBorder(this,&virtwnd,&rc);
		}
		// 
	p.m_pDrawManager->DrawEnd(this,&virtwnd);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::ConvertPaneToLine(HPANE pane, bool horz)
{	assert( GetSafeHwnd() );
	assert(pane==nullptr || IsPaneExist(pane));
	assert( !IsLine(pane) );
		// 
	HPANE h = p.m_Tree.Add(pane);
		// 
	if( !p.CreatePane(h,false) )
	{	p.m_Tree.Delete(h);
		throw std::bad_alloc();
	}
		// 
	Private::Pane *parent = p.m_Tree.GetData(pane);
	p.m_Tree.GetData(h)->tab = parent->tab;
	parent->tab = nullptr;
	parent->horz = horz;
		// 
	return h;
}
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::AddPane(HPANE parent)
{	assert( GetSafeHwnd() );
	assert(parent==nullptr || IsPaneExist(parent));
	assert( IsLine(parent) );
		// 
	HPANE h = p.m_Tree.Add(parent);
		// 
	if( !p.CreatePane(h,true) )
	{	p.m_Tree.Delete(h);
		throw std::bad_alloc();
	}
		// 
	Private::Pane *prev = p.m_Tree.GetData( p.m_Tree.GetPrevChild(h) );
	Private::Pane *pane = p.m_Tree.GetData(h);
	pane->real.factor = prev->real.factor / 2.0;
	prev->real.factor -= pane->real.factor;
		// 
	return h;
}
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::InsertPane(HPANE before, Space space)
{	assert( GetSafeHwnd() );
	assert( IsPaneExist(before) );
	assert( GetParentPane(before) );
		// 
	HPANE h = p.m_Tree.Insert(before);
		// 
	if( !p.CreatePane(h,true) )
	{	p.m_Tree.Delete(h);
		throw std::bad_alloc();
	}
		// 
	Private::Pane *pn = p.m_Tree.GetData(h);
	Private::Pane *pNext = p.m_Tree.GetData(before);
		// 
	if(p.m_Tree.GetPrevChild(h)==nullptr)   // inserted as first pane.
	{	pn->real.factor = pNext->real.factor/2.0;
		pNext->real.factor -= pn->real.factor;
	}
	else
	{	Private::Pane *pPrev = p.m_Tree.GetData( p.m_Tree.GetPrevChild(h) );
			// 
		switch(space)
		{	case SpaceBetween:
			{	const double totalfactor = pPrev->real.factor + pNext->real.factor;
				pPrev->real.factor *= 2.0/3.0;
				pNext->real.factor *= 2.0/3.0;
				pn->real.factor = totalfactor - pPrev->real.factor - pNext->real.factor;
			}	break;
				// 
			case SpacePrevious:
				pn->real.factor = pPrev->real.factor/2.0;
				pPrev->real.factor -= pn->real.factor;
				break;
				// 
			case SpaceNext:
				pn->real.factor = pNext->real.factor/2.0;
				pNext->real.factor -= pn->real.factor;
				break;
		}
	}
		// 
	return h;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::RemovePane(HPANE before, HPANE src)
{	assert( GetSafeHwnd() );
	assert(before!=src);
	assert(IsPaneExist(before) && IsPaneExist(src));
	assert(GetParentPane(before)==GetParentPane(src));
		// 
	p.m_Tree.Remove(before,src);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::DeletePane(HPANE pane, Space space)
{	assert( GetSafeHwnd() );
	assert( IsPaneExist(pane) );
	assert( GetParentPane(pane) );
		// 
	HPANE prev = p.m_Tree.GetPrevChild(pane);
	HPANE next = p.m_Tree.GetNextChild(pane);
		// 
	Private::Pane *pPrev = (prev ? p.m_Tree.GetData(prev) : nullptr);
	Private::Pane *pn = p.m_Tree.GetData(pane);
	Private::Pane *pNext = (next ? p.m_Tree.GetData(next) : nullptr);
		// 
	if(pPrev==nullptr)   // first pane.
	{	if(pNext)
			pNext->real.factor += pn->real.factor;
	}
	else if(pNext==nullptr)   // last pane.
	{	if(pPrev)
			pPrev->real.factor += pn->real.factor;
	}
	else
		switch(space)
		{	case SpaceBetween:
			{	const double totalfactor = pPrev->real.factor + pn->real.factor + pNext->real.factor;
				pNext->real.factor = (pNext->real.factor * totalfactor) / (pPrev->real.factor + pNext->real.factor);
				pPrev->real.factor = totalfactor - pNext->real.factor;
			}	break;
				// 
			case SpacePrevious:
				pPrev->real.factor += pn->real.factor;
				break;
				// 
			case SpaceNext:
				pNext->real.factor += pn->real.factor;
				break;
		}
		// 
	p.m_Tree.Delete(pane);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::DeleteAllPanes()
{	assert( GetSafeHwnd() );
		// 
	p.m_Tree.DeleteAll();
		// 
	Private::Pane *rootPane = p.m_Tree.GetData(nullptr);
	rootPane->tab->DeleteAllTabs();   // delete all tabs in the root pane.
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::InsertIntermediatePane(HPANE parent, bool horz)
{	assert( GetSafeHwnd() );
	assert(parent==nullptr || IsPaneExist(parent));
	assert( IsLine(parent) );
		// 
	HPANE h = p.m_Tree.InsertIntermediate(parent);
		// 
	if( !p.CreatePane(h,false) )
	{	p.m_Tree.DeleteIntermediate(h);
		throw std::bad_alloc();
	}
		// 
	p.m_Tree.GetData(h)->horz = horz;
	return h;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::DeleteIntermediatePane(HPANE pane)
{	assert( GetSafeHwnd() );
	assert( IsPaneExist(pane) );
	assert( IsLine(pane) );
	assert( GetParentPane(pane) );
	assert(GetNumberPanes(GetParentPane(pane))==1);
		// 
	SetLineOrientation(GetParentPane(pane),IsLineHorizontal(pane));   // copy orientation to parent.
	p.m_Tree.DeleteIntermediate(pane);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SetDockingMarkers(DockingMarkers::Layout const &layout, int insertMarkerTransp/*1...100%*/)
{	if( p.m_DockMarkMngr.IsCreated() )
		p.m_DockMarkMngr.Destroy();
	return p.m_DockMarkMngr.Create(layout,insertMarkerTransp);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::DisableDockingMarkers()
{	p.m_DockMarkMngr.Destroy();
}
// 
bool MultiPaneCtrl::IsDockingMarkersEnable() const
{	return p.m_DockMarkMngr.IsCreated();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::CreateSystemImages(HMODULE moduleRes/*or null*/, UINT resID/*or 0*/, bool pngImage, int imageWidth, COLORREF clrTransp/*=CLR_NONE*/)
{	assert(!resID || imageWidth>0);
		// 
	if(p.m_ImageSys.bmp)
	{	::delete p.m_ImageSys.bmp;
		p.m_ImageSys.bmp = nullptr;
	}
		// 
	const bool res = (!resID || p.LoadImage(moduleRes,resID,pngImage,&p.m_ImageSys.bmp/*out*/));
		// 
	(p.m_ImageSys.bmp ?
		p.m_ImageSys.size.SetSize(imageWidth, p.m_ImageSys.bmp->GetHeight() ) :
		p.m_ImageSys.size.SetSize(0,0));
	p.m_clrImageSysTransp = clrTransp;
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
			{	TabCtrl *ctrl = GetTabCtrl(h);
				ctrl->SetSystemImagesRef(p.m_ImageSys.bmp,imageWidth,clrTransp);
			}
		// 
	return res;
}
//
Gdiplus::Bitmap *MultiPaneCtrl::GetSystemImages() const
{	return p.m_ImageSys.bmp;
}
// 
CSize MultiPaneCtrl::GetSystemImageSize() const
{	return p.m_ImageSys.size;
}
// 
COLORREF MultiPaneCtrl::GetSystemImagesTranspColor() const
{	return p.m_clrImageSysTransp;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
bool MultiPaneCtrl::CreateImages(HMODULE moduleRes/*or null*/, UINT resNormalID/*or 0*/, UINT resDisableID/*or 0*/, bool pngImage, int imageWidth, COLORREF clrTransp/*=CLR_NONE*/)
{	assert((!resNormalID && !resDisableID) || imageWidth>0);
		// 
	if(p.m_ImageNormal.bmp)
	{	::delete p.m_ImageNormal.bmp;
		p.m_ImageNormal.bmp = nullptr;
	}
	if(p.m_ImageDisable.bmp)
	{	::delete p.m_ImageDisable.bmp;
		p.m_ImageDisable.bmp = nullptr;
	}
		// 
	bool res = true;
	if(resNormalID)
		if( !p.LoadImage(moduleRes,resNormalID,pngImage,&p.m_ImageNormal.bmp/*out*/) )
			res = false;
	if(resDisableID)
		if( !p.LoadImage(moduleRes,resDisableID,pngImage,&p.m_ImageDisable.bmp/*out*/) )
			res = false;
		// 
	(p.m_ImageNormal.bmp ?
		p.m_ImageNormal.size.SetSize(imageWidth, p.m_ImageNormal.bmp->GetHeight() ) :
		p.m_ImageNormal.size.SetSize(0,0));
	(p.m_ImageDisable.bmp ?
		p.m_ImageDisable.size.SetSize(imageWidth, p.m_ImageDisable.bmp->GetHeight() ) :
		p.m_ImageDisable.size.SetSize(0,0));
	p.m_clrImageTransp = clrTransp;
		//
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
			{	TabCtrl *ctrl = GetTabCtrl(h);
				ctrl->SetImagesRef(p.m_ImageNormal.bmp,p.m_ImageDisable.bmp,imageWidth,clrTransp);
			}
		// 
	return res;
}
//
void MultiPaneCtrl::GetImages(Gdiplus::Bitmap **normal/*out,or null*/, Gdiplus::Bitmap **disable/*out,or null*/) const
{	if(normal)
		*normal = p.m_ImageNormal.bmp;
	if(disable)
		*disable = p.m_ImageDisable.bmp;
}
// 
void MultiPaneCtrl::GetImageSize(CSize *szNormal/*out,or null*/, CSize *szDisable/*out,or null*/) const
{	if(szNormal)
	{	szNormal->cx = p.m_ImageNormal.size.cx;
		szNormal->cy = p.m_ImageNormal.size.cy;
	}
	if(szDisable)
	{	szDisable->cx = p.m_ImageDisable.size.cx;
		szDisable->cy = p.m_ImageDisable.size.cy;
	}
}
//
COLORREF MultiPaneCtrl::GetImagesTranspColor() const
{	return p.m_clrImageTransp;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SetCursors(UINT tab/*or 0*/, UINT splitterHorz/*or 0*/, UINT splitterVert/*or 0*/, UINT dragEnable/*or 0*/, UINT dragDisable/*or 0*/)
{	return SetCursors( AfxGetResourceHandle(), tab,splitterHorz,splitterVert,dragEnable,dragDisable);
}
// 
bool MultiPaneCtrl::SetCursors(HMODULE module, UINT tab/*or 0*/, UINT splitterHorz/*or 0*/, UINT splitterVert/*or 0*/, UINT dragEnable/*or 0*/, UINT dragDisable/*or 0*/)
{	if(p.m_hCurTab)
	{	::DestroyCursor(p.m_hCurTab);
		p.m_hCurTab = nullptr;
	}
	p.m_hpCurTabRef = nullptr;
		// 
	if(p.m_hCurSplitterHorz)
	{	::DestroyCursor(p.m_hCurSplitterHorz);
		p.m_hCurSplitterHorz = nullptr;
	}
	p.m_hpCurSplitterHorzRef = nullptr;
		// 
	if(p.m_hCurSplitterVert)
	{	::DestroyCursor(p.m_hCurSplitterVert);
		p.m_hCurSplitterVert = nullptr;
	}
	p.m_hpCurSplitterVertRef = nullptr;
		// 
	if(p.m_hCurDragEnable)
	{	::DestroyCursor(p.m_hCurDragEnable);
		p.m_hCurDragEnable = nullptr;
	}
	p.m_hpCurDragEnableRef = nullptr;
		// 
	if(p.m_hCurDragDisable)
	{	::DestroyCursor(p.m_hCurDragDisable);
		p.m_hCurDragDisable = nullptr;
	}
	p.m_hpCurDragDisableRef = nullptr;
		// 
	bool res = true;
		// 
	if(module)
	{	if(tab)
			if((p.m_hCurTab = ::LoadCursor(module,MAKEINTRESOURCE(tab)))!=nullptr)
				p.m_hpCurTabRef = &p.m_hCurTab;
			else
				res = false;
			// 
		if(splitterHorz)
			if((p.m_hCurSplitterHorz = ::LoadCursor(module,MAKEINTRESOURCE(splitterHorz)))!=nullptr)
				p.m_hpCurSplitterHorzRef = &p.m_hCurSplitterHorz;
			else
				res = false;
			// 
		if(splitterVert)
			if((p.m_hCurSplitterVert = ::LoadCursor(module,MAKEINTRESOURCE(splitterVert)))!=nullptr)
				p.m_hpCurSplitterVertRef = &p.m_hCurSplitterVert;
			else
				res = false;
			// 
		if(dragEnable)
			if((p.m_hCurDragEnable = ::LoadCursor(module,MAKEINTRESOURCE(dragEnable)))!=nullptr)
				p.m_hpCurDragEnableRef = &p.m_hCurDragEnable;
			else
				res = false;
			// 
		if(dragDisable)
			if((p.m_hCurDragDisable = ::LoadCursor(module,MAKEINTRESOURCE(dragDisable)))!=nullptr)
				p.m_hpCurDragDisableRef = &p.m_hCurDragDisable;
			else
				res = false;
	}
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if( GetNumberPanes(h)==0 )
			{	TabCtrl *tabCtrl = GetTabCtrl(h);
				tabCtrl->SetCursorRef(p.m_hpCurTabRef);
			}
		// 
	return res;
}
// 
bool MultiPaneCtrl::SetCursors(HCURSOR tab/*or null*/, HCURSOR splitterHorz/*or null*/, HCURSOR splitterVert/*or null*/, HCURSOR dragEnable/*or null*/, HCURSOR dragDisable/*or null*/)
{	if(p.m_hCurTab)
	{	::DestroyCursor(p.m_hCurTab);
		p.m_hCurTab = nullptr;
	}
	p.m_hpCurTabRef = nullptr;
		// 
	if(p.m_hCurSplitterHorz)
	{	::DestroyCursor(p.m_hCurSplitterHorz);
		p.m_hCurSplitterHorz = nullptr;
	}
	p.m_hpCurSplitterHorzRef = nullptr;
		// 
	if(p.m_hCurSplitterVert)
	{	::DestroyCursor(p.m_hCurSplitterVert);
		p.m_hCurSplitterVert = nullptr;
	}
	p.m_hpCurSplitterVertRef = nullptr;
		// 
	if(p.m_hCurDragEnable)
	{	::DestroyCursor(p.m_hCurDragEnable);
		p.m_hCurDragEnable = nullptr;
	}
	p.m_hpCurDragEnableRef = nullptr;
		// 
	if(p.m_hCurDragDisable)
	{	::DestroyCursor(p.m_hCurDragDisable);
		p.m_hCurDragDisable = nullptr;
	}
	p.m_hpCurDragDisableRef = nullptr;
		// 
	bool res = true;
		// 
	if(tab)
		if((p.m_hCurTab = static_cast<HCURSOR>( CopyImage(tab,IMAGE_CURSOR,0,0,0) ))!=nullptr)
			p.m_hpCurTabRef = &p.m_hCurTab;
		else
			res = false;
		// 
	if(splitterHorz)
		if((p.m_hCurSplitterHorz = static_cast<HCURSOR>( CopyImage(splitterHorz,IMAGE_CURSOR,0,0,0) ))!=nullptr)
			p.m_hpCurSplitterHorzRef = &p.m_hCurSplitterHorz;
		else
			res = false;
		// 
	if(splitterVert)
		if((p.m_hCurSplitterVert = static_cast<HCURSOR>( CopyImage(splitterVert,IMAGE_CURSOR,0,0,0) ))!=nullptr)
			p.m_hpCurSplitterVertRef = &p.m_hCurSplitterVert;
		else
			res = false;
		// 
	if(dragEnable)
		if((p.m_hCurDragEnable = static_cast<HCURSOR>( CopyImage(dragEnable,IMAGE_CURSOR,0,0,0) ))!=nullptr)
			p.m_hpCurDragEnableRef = &p.m_hCurDragEnable;
		else
			res = false;
		// 
	if(dragDisable)
		if((p.m_hCurDragDisable = static_cast<HCURSOR>( CopyImage(dragDisable,IMAGE_CURSOR,0,0,0) ))!=nullptr)
			p.m_hpCurDragDisableRef = &p.m_hCurDragDisable;
		else
			res = false;
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
			{	TabCtrl *tabCtrl = GetTabCtrl(h);
				tabCtrl->SetCursorRef(p.m_hpCurTabRef);
			}
		// 
	return res;
}
// 
void MultiPaneCtrl::SetCursorsRef(HCURSOR *tab/*or null*/, HCURSOR *splitterHorz/*or null*/, HCURSOR *splitterVert/*or null*/, HCURSOR *dragEnable/*or null*/, HCURSOR *dragDisable/*or null*/)
{	if(p.m_hCurTab)
	{	::DestroyCursor(p.m_hCurTab);
		p.m_hCurTab = nullptr;
	}
	p.m_hpCurTabRef = tab;
		// 
	if(p.m_hCurSplitterHorz)
	{	::DestroyCursor(p.m_hCurSplitterHorz);
		p.m_hCurSplitterHorz = nullptr;
	}
	p.m_hpCurSplitterHorzRef = splitterHorz;
		// 
	if(p.m_hCurSplitterVert)
	{	::DestroyCursor(p.m_hCurSplitterVert);
		p.m_hCurSplitterVert = nullptr;
	}
	p.m_hpCurSplitterVertRef = splitterVert;
		// 
	if(p.m_hCurDragEnable)
	{	::DestroyCursor(p.m_hCurDragEnable);
		p.m_hCurDragEnable = nullptr;
	}
	p.m_hpCurDragEnableRef = dragEnable;
		// 
	if(p.m_hCurDragDisable)
	{	::DestroyCursor(p.m_hCurDragDisable);
		p.m_hCurDragDisable = nullptr;
	}
	p.m_hpCurDragDisableRef = dragDisable;
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
			{	TabCtrl *tabCtrl = GetTabCtrl(h);
				tabCtrl->SetCursorRef(p.m_hpCurTabRef);
			}
}
// 
void MultiPaneCtrl::GetCursors(HCURSOR *tab/*out,or null*/, HCURSOR *splitterHorz/*out,or null*/, HCURSOR *splitterVert/*out,or null*/, 
	HCURSOR *dragEnable/*out,or null*/, HCURSOR *dragDisable/*out,or null*/) const
{
	if(tab) 
		*tab = (p.m_hpCurTabRef ? *p.m_hpCurTabRef : nullptr);
	if(splitterHorz) 
		*splitterHorz = (p.m_hpCurSplitterHorzRef ? *p.m_hpCurSplitterHorzRef : nullptr);
	if(splitterVert) 
		*splitterVert = (p.m_hpCurSplitterVertRef ? *p.m_hpCurSplitterVertRef : nullptr);
	if(dragEnable) 
		*dragEnable = (p.m_hpCurDragEnableRef ? *p.m_hpCurDragEnableRef : nullptr);
	if(dragDisable) 
		*dragDisable = (p.m_hpCurDragDisableRef ? *p.m_hpCurDragDisableRef : nullptr);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SetFontNormal(CFont *font)
{	assert(font && font->m_hObject);
		// 
	LOGFONT logfont;
	font->GetLogFont(&logfont/*out*/);
	return SetFontNormal(&logfont);
}
// 
bool MultiPaneCtrl::SetFontNormal(LOGFONT const *lf)
{	assert(lf);
		// 
	if(p.m_Font.m_hObject)
		p.m_Font.DeleteObject();
	p.m_pFontRef = nullptr;
		// 
	const bool res = (p.m_Font.CreateFontIndirect(lf)!=0);
	if(res)
		p.m_pFontRef = &p.m_Font;
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
			{	TabCtrl *tab = GetTabCtrl(h);
				if(tab)
					tab->SetFontNormalRef(p.m_pFontRef);
			}
		// 
	return res;
}
// 
void MultiPaneCtrl::SetFontNormalRef(CFont *font)
{	assert(font && font->m_hObject);
		// 
	if(p.m_Font.m_hObject)
		p.m_Font.DeleteObject();
	p.m_pFontRef = font;
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
			{	TabCtrl *tab = GetTabCtrl(h);
				if(tab)
					tab->SetFontNormalRef(p.m_pFontRef);
			}
}
// 
CFont *MultiPaneCtrl::GetFontNormal()
{	return p.m_pFontRef;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SetFontSelect(CFont *font)
{	assert(font && font->m_hObject);
		// 
	LOGFONT logfont;
	font->GetLogFont(&logfont/*out*/);
	return SetFontSelect(&logfont);
}
// 
bool MultiPaneCtrl::SetFontSelect(LOGFONT const *lf)
{	assert(lf);
		// 
	if(p.m_FontSelect.m_hObject)
		p.m_FontSelect.DeleteObject();
	p.m_pFontSelectRef = nullptr;
		// 
	const bool res = (p.m_FontSelect.CreateFontIndirect(lf)!=0);
	if(res)
		p.m_pFontSelectRef = &p.m_FontSelect;
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
			{	TabCtrl *tab = GetTabCtrl(h);
				if(tab)
					tab->SetFontSelectRef(p.m_pFontSelectRef);
			}
		// 
	return res;
}
// 
void MultiPaneCtrl::SetFontSelectRef(CFont *font)
{	assert(font && font->m_hObject);
		// 
	if(p.m_FontSelect.m_hObject) 
		p.m_FontSelect.DeleteObject();
	p.m_pFontSelectRef = font;
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
			{	TabCtrl *tab = GetTabCtrl(h);
				if(tab)
					tab->SetFontSelectRef(p.m_pFontSelectRef);
			}
}
// 
CFont *MultiPaneCtrl::GetFontSelect()
{	return p.m_pFontSelectRef;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetPaneData(HPANE pane, __int64 data)
{	assert( GetSafeHwnd() );
	assert(pane==nullptr || IsPaneExist(pane));
		// 
	p.m_Tree.GetData(pane)->data = data;
}
// 
__int64 MultiPaneCtrl::GetPaneData(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert(pane==nullptr || IsPaneExist(pane));
		// 
	return p.m_Tree.GetData(pane)->data;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetRootPane() const
{	assert( GetSafeHwnd() );
		// 
	return p.m_Tree.GetRoot();
}
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetParentPane(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert( IsPaneExist(pane) );
		// 
	return p.m_Tree.GetParent(pane);
}
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetPaneHandleByIndex(HPANE parent, int idx) const
{	assert( GetSafeHwnd() );
	assert(parent==nullptr || IsPaneExist(parent));
	assert(idx>=0 && idx<GetNumberPanes(parent));
		// 
	return p.m_Tree.GetNode(parent,idx);
}
/////////////////////////////////////////////////////////////////////////////
// 
int MultiPaneCtrl::GetPaneIndexByHandle(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert( IsPaneExist(pane) );
	assert(pane!=GetRootPane());
		// 
	return p.m_Tree.GetIndex(pane);
}
/////////////////////////////////////////////////////////////////////////////
// 
int MultiPaneCtrl::GetNumberPanes(HPANE parent) const
{	assert( GetSafeHwnd() );
	assert(parent==nullptr || IsPaneExist(parent));
		// 
	return p.m_Tree.GetNumber(parent);
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::IsPaneExist(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert(pane);
		// 
	return p.m_Tree.IsExist(pane);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetFirstChildPane(HPANE parent) const
{	assert( GetSafeHwnd() );
	assert(parent==nullptr || IsPaneExist(parent));
		// 
	return p.m_Tree.GetFirstChild(parent);
}
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetPrevChildPane(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert( IsPaneExist(pane) );
	assert(pane!=GetRootPane());
		// 
	return p.m_Tree.GetPrevChild(pane);
}
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetNextChildPane(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert( IsPaneExist(pane) );
	assert(pane!=GetRootPane());
		// 
	return p.m_Tree.GetNextChild(pane);
}
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetLastChildPane(HPANE parent) const
{	assert( GetSafeHwnd() );
	assert(parent==nullptr || IsPaneExist(parent));
		// 
	return p.m_Tree.GetLastChild(parent);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetFirstPane() const
{	assert( GetSafeHwnd() );
		// 
	return p.m_Tree.GetFirst();
}
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetPrevPane(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert( IsPaneExist(pane) );
		// 
	return p.m_Tree.GetPrev(pane);
}
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetNextPane(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert( IsPaneExist(pane) );
		// 
	return p.m_Tree.GetNext(pane);
}
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetLastPane() const
{	assert( GetSafeHwnd() );
		// 
	return p.m_Tree.GetLast();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::HitTest(CPoint ptScr) const
{	assert( GetSafeHwnd() );
		// 
	CRect rc;
		// 
	for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
		if(GetNumberPanes(h)==0)   // has tabs.
		{	TabCtrl *tabCtrl = GetTabCtrl(h);
			tabCtrl->GetWindowRect(&rc);
			if( rc.PtInRect(ptScr) )
				return h;
		}
	return nullptr;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::IsLine(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert(pane==nullptr || IsPaneExist(pane));
		// 
	return GetNumberPanes(pane)>0;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::IsLineHorizontal(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert( IsLine(pane) );
		// 
	return p.m_Tree.GetData(pane)->horz;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetLineOrientation(HPANE pane, bool horz)
{	assert( GetSafeHwnd() );
	assert( IsLine(pane) );
		// 
	p.m_Tree.GetData(pane)->horz = horz;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::ShowBorder(bool show)
{	p.m_bShowBorder = show;
}
// 
bool MultiPaneCtrl::IsBorderVisible() const
{	return p.m_bShowBorder;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::EnableTabDrag(bool enable)
{	p.m_bDragTabEnable = enable;
}
bool MultiPaneCtrl::IsTabDragEnable() const
{	return p.m_bDragTabEnable;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::EnableDropOptimization(bool optimiz)
{	p.m_bDropOptimiz = optimiz;
}
// 
bool MultiPaneCtrl::IsDropOptimizationEnable() const
{	return p.m_bDropOptimiz;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetMinTargetSizeForDrop(CSize const &size)
{	assert(size.cx>=0 && size.cy>=0);
		// 
	p.m_szMinSizeForDrop = size;
}
// 
CSize MultiPaneCtrl::GetMinTargetSizeForDrop() const
{	return p.m_szMinSizeForDrop;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetLinePortions(HPANE parent, int *percent/*in*/)
{	assert( GetSafeHwnd() );
	assert(parent==nullptr || IsPaneExist(parent));
	assert( IsLine(parent) );
	assert(percent);
		// 
	const int count = GetNumberPanes(parent);
		// 
	__try
	{	int sum = 0;
		for(int c=0; c<count; ++c)
			sum += percent[c];
			// 
		assert(sum==100);
		if(sum!=100) return;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{	assert(!"count items in 'percent' != GetNumber(parent)");
		return;
	}
		// 
	int i = 0;
	for(HPANE h=GetFirstChildPane(parent); h; h=GetNextChildPane(h), ++i)
	{	const double factor = static_cast<double>(percent[i])/100.0;
		p.m_Tree.GetData(h)->real.factor = factor;
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
CRect MultiPaneCtrl::GetPaneRect(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert(pane==nullptr || IsPaneExist(pane));
		// 
	return p.m_Tree.GetData(pane)->real.rc;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetPanePortion(HPANE pane, double portion, Space space) const
{	assert( GetSafeHwnd() );
	assert( IsPaneExist(pane) );
	assert( GetParentPane(pane) );
	assert(portion>0.0 && portion<1.0);
		// 
	HPANE prev = p.m_Tree.GetPrevChild(pane);
	HPANE next = p.m_Tree.GetNextChild(pane);
		// 
	Private::Pane *pPrev = (prev ? p.m_Tree.GetData(prev) : nullptr);
	Private::Pane *pn = p.m_Tree.GetData(pane);
	Private::Pane *pNext = (next ? p.m_Tree.GetData(next) : nullptr);
		// 
	if(pPrev==nullptr)   // first pane.
	{	if(pNext)
		{	const double totalfactor = pNext->real.factor + pn->real.factor;
				// 
			if(portion < totalfactor)
			{	pNext->real.factor = totalfactor - portion;
				pn->real.factor = portion;
			}
		}
	}
	else if(pNext==nullptr)   // last pane.
	{	if(pPrev)
		{	const double totalfactor = pPrev->real.factor + pn->real.factor;
				// 
			if(portion < totalfactor)
			{	pPrev->real.factor = totalfactor - portion;
				pn->real.factor = portion;
			}
		}
	}
	else   // intermediate pane.
		switch(space)
		{	case SpaceBetween:
			{	const double totalfactor = pPrev->real.factor + pn->real.factor + pNext->real.factor;
					// 
				if(portion < totalfactor)
				{	pPrev->real.factor = pPrev->real.factor * (totalfactor - portion) / (totalfactor - pn->real.factor);
					pNext->real.factor = totalfactor - pPrev->real.factor - portion;
					pn->real.factor = portion;
				}
			}	break;
				// 
			case SpacePrevious:
			{	const double totalfactor = pPrev->real.factor + pn->real.factor;
					// 
				if(portion < totalfactor)
				{	pPrev->real.factor = totalfactor - portion;
					pn->real.factor = portion;
				}
			}	break;
				// 
			case SpaceNext:
			{	const double totalfactor = pNext->real.factor + pn->real.factor;
					// 
				if(portion < totalfactor)
				{	pNext->real.factor = totalfactor - portion;
					pn->real.factor = portion;
				}
			}	break;
		}
}
/////////////////////////////////////////////////////////////////////////////
// 
double MultiPaneCtrl::GetPanePortion(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert(pane==nullptr || IsPaneExist(pane));
		// 
	return p.m_Tree.GetData(pane)->real.factor;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::CopyLinePortions(HPANE dst, MultiPaneCtrl const *paneCtrlSrc, HPANE src)
{	assert( GetSafeHwnd() );
	assert(dst==nullptr || IsPaneExist(dst));
	assert( IsLine(dst) );
	assert(paneCtrlSrc && paneCtrlSrc->IsPaneExist(src));
	assert( paneCtrlSrc->IsLine(src) );
	assert(GetNumberPanes(dst)==paneCtrlSrc->GetNumberPanes(src));
		// 
	for(int i=0, c=GetNumberPanes(dst); i<c; ++i)
	{	HPANE d = GetPaneHandleByIndex(dst,i);
		HPANE s = paneCtrlSrc->GetPaneHandleByIndex(src,i);
		p.m_Tree.GetData(d)->real.factor = paneCtrlSrc->GetPanePortion(s);
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetLineEqualPanesSize(HPANE parent)
{	assert( GetSafeHwnd() );
	assert(parent==nullptr || IsPaneExist(parent));
	assert( IsLine(parent) );
		// 
	const double f = 1.0 / static_cast<double>( GetNumberPanes(parent) );
	for(HPANE h=GetFirstChildPane(parent); h; h=GetNextChildPane(h))
		p.m_Tree.GetData(h)->real.factor = f;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetLinesEqualPanesSize()
{	assert( GetSafeHwnd() );
		// 
	for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
		if( IsLine(h) )
			SetLineEqualPanesSize(h);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetPanesMinSize(CSize const &size)
{	assert(size.cx>=0 && size.cy>=0);
		// 
	p.m_szMinSize.cx = size.cx;
	p.m_szMinSize.cy = size.cy;
}
// 
CSize MultiPaneCtrl::GetPanesMinSize() const
{	return p.m_szMinSize;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::WatchCtrlActivity(bool watch)
{	if(watch!=IsWatchCtrlActivity())
	{	p.m_TabState.WatchCtrlActivity(watch);
		p.m_bActive = false;
			// 
		if( GetSafeHwnd() )
		{	if(watch)   // on.
				p.m_ActivityHook.Add(m_hWnd, &p,&Private::OnActive);
			else   // off.
				p.m_ActivityHook.Delete(m_hWnd);
				// 
			for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
				if(GetNumberPanes(h)==0)
					GetTabCtrl(h)->WatchCtrlActivity(watch);
		}
	}
}
// 
bool MultiPaneCtrl::IsWatchCtrlActivity() const
{	return p.m_TabState.IsWatchCtrlActivity();
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::IsActive() const
{	return p.m_bActive;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetSplitterDraggingMode(SplitterDragging mode)
{	p.m_SplitterDragMode = mode;
}
// 
MultiPaneCtrl::SplitterDragging MultiPaneCtrl::GetSplitterDraggingMode() const
{	return p.m_SplitterDragMode;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::IsSplitterDragging(bool *horz/*out*/) const
{	assert( GetSafeHwnd() );
		// 
	if(p.m_hSplitDragPane)
	{	if(horz)
			*horz = p.IsSplitterHorizontal(p.m_hSplitDragPane);
		return true;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::CancelDragging()
{	assert( GetSafeHwnd() );
		// 
	p.StopSplitterDragging(true);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::ActivateSplitter(HPANE pane, bool active)
{	assert( GetSafeHwnd() );
	assert( IsPaneExist(pane) );
	assert( GetParentPane(pane) );
	assert( GetLastChildPane(GetParentPane(pane))!=pane );   // it isn't last pane in line.
		// 
	p.m_Tree.GetData(pane)->activeSplitter = active;
}
// 
bool MultiPaneCtrl::IsSplitterActive(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert( IsPaneExist(pane) );
	assert( GetParentPane(pane) );
	assert( GetLastChildPane(GetParentPane(pane))!=pane );   // it isn't last pane in line.
		// 
	return p.m_Tree.GetData(pane)->activeSplitter;
}
/////////////////////////////////////////////////////////////////////////////
// 
CRect MultiPaneCtrl::GetSplitterRect(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert( IsPaneExist(pane) );
	assert( GetParentPane(pane) );
	assert( GetLastChildPane(GetParentPane(pane))!=pane );   // it isn't last pane in line.
		// 
	return p.m_Tree.GetData(pane)->real.rcSplitter;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
TabCtrl::HTAB MultiPaneCtrl::AddTab(HPANE pane, HWND wnd, TCHAR const *text, int image)
{	assert( GetSafeHwnd() );
	assert(pane==nullptr || IsPaneExist(pane));
	assert(!IsLine(pane) && GetTabCtrl(pane));
	assert(wnd && ::IsWindow(wnd));
		// 
	HWND hParentOld = ::GetParent(wnd);
		// 
	TabCtrl *tab = GetTabCtrl(pane);
	::SetParent(wnd,tab->m_hWnd);
		// 
	try
	{	return tab->AddTab(wnd,text,image);
	}
	catch(std::bad_alloc &)
	{	::SetParent(wnd,hParentOld);
		throw;
	}
}
// 
TabCtrl::HTAB MultiPaneCtrl::AddTab(HPANE pane, ITab const *tab)
{	assert( GetSafeHwnd() );
	assert(tab);
		// 
	return AddTab(pane,tab->GetWnd(),tab->GetText(),tab->GetImage());
}
/////////////////////////////////////////////////////////////////////////////
// 
TabCtrl::HTAB MultiPaneCtrl::InsertTab(HPANE pane, TabCtrl::HTAB before, HWND wnd, TCHAR const *text, int image)
{	assert( GetSafeHwnd() );
	assert(pane==nullptr || IsPaneExist(pane));
	assert( !IsLine(pane) );
	assert(GetTabCtrl(pane)->IsTabExist(before));
	assert(wnd && ::IsWindow(wnd));
		// 
	HWND hParentOld = ::GetParent(wnd);
		// 
	TabCtrl *tab = GetTabCtrl(pane);
	::SetParent(wnd,tab->m_hWnd);
		// 
	try
	{	return tab->InsertTab(before,wnd,text,image);
	}
	catch(std::bad_alloc &)
	{	::SetParent(wnd,hParentOld);
		throw;
	}
}
// 
TabCtrl::HTAB MultiPaneCtrl::InsertTab(HPANE pane, TabCtrl::HTAB before, ITab const *tab)
{	assert( GetSafeHwnd() );
	assert(tab);
		// 
	return InsertTab(pane,before,tab->GetWnd(),tab->GetText(),tab->GetImage());
}
/////////////////////////////////////////////////////////////////////////////
// 
TabCtrl *MultiPaneCtrl::GetTabCtrl(HPANE pane) const
{	assert( GetSafeHwnd() );
	assert(pane==nullptr || IsPaneExist(pane));
	assert( !IsLine(pane) );
		// 
	return p.m_Tree.GetData(pane)->tab;
}
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetPaneWithTabCtrl(TabCtrl const *ctrl) const
{	assert( GetSafeHwnd() );
	assert(ctrl);
		// 
	for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
		if(GetNumberPanes(h)==0)
			if(GetTabCtrl(h)==ctrl)
				return h;
	return nullptr;
}
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetPaneWithTab(TabCtrl::HTAB tab) const
{	assert( GetSafeHwnd() );
	assert(tab);
		// 
	for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
		if(GetNumberPanes(h)==0)
		{	TabCtrl *tabCtrl = GetTabCtrl(h);
			if( tabCtrl->IsTabExist(tab) )
				return h;
		}
	return nullptr;
}
/////////////////////////////////////////////////////////////////////////////
// 
TabCtrl::HTAB MultiPaneCtrl::GetTabWithWindowID(int id, HPANE *pane/*out,or null*/) const
{	assert( GetSafeHwnd() );
		// 
	for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
		if(GetNumberPanes(h)==0)
		{	TabCtrl const *tabCtrl = GetTabCtrl(h);
			TabCtrl::HTAB tab = tabCtrl->GetTabWithWindowID(id);
			if(tab)
			{	if(pane)
					*pane = h;
				return tab;
			}
		}
	return nullptr;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::IsSinglePaneWithTabs() const
{	assert( GetSafeHwnd() );
		// 
	int count = 0;
	for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
		if(GetNumberPanes(h)==0)
			if(++count==2)
				return false;
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetFirstPaneWithTabs() const
{	for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
		if(GetNumberPanes(h)==0)
			return h;
	return nullptr;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::ActivateTab(HPANE pane)
{	assert( GetSafeHwnd() );
	assert(pane==nullptr || IsPaneExist(pane));
	assert(!IsLine(pane) && GetTabCtrl(pane));
		// 
	TabCtrl *tabCtrl = GetTabCtrl(pane);
	TabCtrl::HTAB tab = tabCtrl->GetSelectedTab();
		// 
	if(tab)
	{	HWND wnd = tabCtrl->GetTabWindow(tab);
		if(wnd && ::IsWindow(wnd))
			if(::GetFocus()!=wnd)
				::SetFocus(wnd);
			// 
		tabCtrl->EnsureTabVisible(tab);
		tabCtrl->Update();
	}
}
// 
void MultiPaneCtrl::ActivateTab(HPANE pane, TabCtrl::HTAB tab)
{	assert( GetSafeHwnd() );
	assert(pane==nullptr || IsPaneExist(pane));
	assert(!IsLine(pane) && GetTabCtrl(pane));
	assert( GetTabCtrl(pane)->IsTabExist(tab) );
		// 
	TabCtrl *tabCtrl = GetTabCtrl(pane);
		// 
	if(tab)
	{	HWND wnd = tabCtrl->GetTabWindow(tab);
		if(wnd && ::IsWindow(wnd))
			if(::GetFocus()!=wnd)
				::SetFocus(wnd);
			// 
		tabCtrl->SelectTab(tab);
		tabCtrl->EnsureTabVisible(tab);
		tabCtrl->Update();
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetTabsLayout(TabCtrl::Layout layout)
{	p.m_TabState.SetLayout(layout);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->SetLayout(layout);
}
// 
TabCtrl::Layout MultiPaneCtrl::GetTabsLayout() const
{	return p.m_TabState.GetLayout();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetTabsBehavior(TabCtrl::Behavior behavior)
{	p.m_TabState.SetBehavior(behavior);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->SetBehavior(behavior);
}
// 
TabCtrl::Behavior MultiPaneCtrl::GetTabsBehavior() const
{	return p.m_TabState.GetBehavior();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetTabsScrollingStep(int step)
{	p.m_TabState.SetTabsScrollingStep(step);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->SetTabsScrollingStep(step);
}
// 
int MultiPaneCtrl::GetTabsScrollingStep() const
{	return p.m_TabState.GetTabsScrollingStep();
}
/////////////////////////////////////////////////////////////////////////////
//
void MultiPaneCtrl::ShowTabsBorder(bool show)
{	p.m_TabState.ShowBorder(show);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->ShowBorder(show);
}
// 
bool MultiPaneCtrl::IsTabsBorderVisible() const
{	return p.m_TabState.IsBorderVisible();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::EqualTabsSize(bool equal)
{	p.m_TabState.EqualTabsSize(equal);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->EqualTabsSize(equal);
}
// 
bool MultiPaneCtrl::IsEqualTabsSize() const
{	return p.m_TabState.IsEqualTabsSize();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::EnableTabRemove(bool remove)
{	p.m_TabState.EnableTabRemove(remove);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->EnableTabRemove(remove);
}
// 
bool MultiPaneCtrl::IsTabRemoveEnable() const
{	return p.m_TabState.IsTabRemoveEnable();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::HideSingleTab(bool hide)
{	p.m_TabState.HideSingleTab(hide);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->HideSingleTab(hide);
}
// 
bool MultiPaneCtrl::IsHideSingleTab() const
{	return p.m_TabState.IsHideSingleTab();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::ShowButtonsClose(bool show)
{	p.m_TabState.ShowButtonClose(show);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->ShowButtonClose(show);
}
// 
bool MultiPaneCtrl::IsButtonsCloseVisible() const
{	return p.m_TabState.IsButtonCloseVisible();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::ShowButtonsMenu(bool show)
{	p.m_TabState.ShowButtonMenu(show);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->ShowButtonMenu(show);
}
// 
bool MultiPaneCtrl::IsButtonsMenuVisible() const
{	return p.m_TabState.IsButtonMenuVisible();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::ShowButtonsScroll(bool show)
{	p.m_TabState.ShowButtonsScroll(show);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->ShowButtonsScroll(show);
}
// 
bool MultiPaneCtrl::IsButtonsScrollVisible() const
{	return p.m_TabState.IsButtonsScrollVisible();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::EnableTabsToolTip(bool enable)
{	p.m_TabState.EnableToolTip(enable);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->EnableToolTip(enable);
}
// 
bool MultiPaneCtrl::IsTabsToolTipEnable() const
{	return p.m_TabState.IsToolTipEnable();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetButtonsCloseToolTipText(TCHAR const *text)
{	p.m_TabState.SetButtonCloseToolTipText(text);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->SetButtonCloseToolTipText(text);
}
// 
CString MultiPaneCtrl::GetButtonsCloseToolTipText() const
{	return p.m_TabState.GetButtonCloseToolTipText();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetButtonsMenuToolTipText(TCHAR const *text)
{	p.m_TabState.SetButtonMenuToolTipText(text);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->SetButtonMenuToolTipText(text);
}
// 
CString MultiPaneCtrl::GetButtonsMenuToolTipText() const
{	return p.m_TabState.GetButtonMenuToolTipText();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetButtonsScrollLeftToolTipText(TCHAR const *text)
{	p.m_TabState.SetButtonScrollLeftToolTipText(text);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->SetButtonScrollLeftToolTipText(text);
}
// 
CString MultiPaneCtrl::GetButtonsScrollLeftToolTipText() const
{	return p.m_TabState.GetButtonScrollLeftToolTipText();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetButtonsScrollRightToolTipText(TCHAR const *text)
{	p.m_TabState.SetButtonScrollRightToolTipText(text);
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
				GetTabCtrl(h)->SetButtonScrollRightToolTipText(text);
}
// 
CString MultiPaneCtrl::GetButtonsScrollRightToolTipText() const
{	return p.m_TabState.GetButtonScrollRightToolTipText();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetDrawManager(Draw *ptr/*or null*/)
{	p.m_pDrawManager = ptr;
}
// 
MultiPaneCtrl::Draw *MultiPaneCtrl::GetDrawManager() const
{	return p.m_pDrawManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetRecalcManager(IRecalc *ptr)
{	p.m_pRecalcManager = (ptr ? ptr : &p);
}
// 
MultiPaneCtrl::IRecalc *MultiPaneCtrl::GetRecalcManager() const
{	return p.m_pRecalcManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetAbilityManager(Ability *ptr)
{	p.m_pAbilityManager = (ptr ? ptr : &p);
}
// 
MultiPaneCtrl::Ability *MultiPaneCtrl::GetAbilityManager() const
{	return p.m_pAbilityManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetNotifyManager(Notify *ptr)
{	p.m_pNotifyManager = ptr;
}
// 
MultiPaneCtrl::Notify *MultiPaneCtrl::GetNotifyManager() const
{	return p.m_pNotifyManager;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::PreCreateTabCtrl(TabCtrl *tab)
{	if(m_pTabStyle)
		m_pTabStyle->Install(tab);
	tab->SetAbilityManager(this);
	tab->SetNotifyManager(this);
		// 
		// 
		// set state.
	tab->SetLayout( m_TabState.GetLayout() );
	tab->SetBehavior( m_TabState.GetBehavior() );
		//
	tab->SetSystemImagesRef(m_ImageSys.bmp,m_ImageSys.size.cx,m_clrImageSysTransp);
	tab->SetImagesRef(m_ImageNormal.bmp,m_ImageDisable.bmp,(m_ImageNormal.bmp ? m_ImageNormal.size.cx : m_ImageDisable.size.cx),m_clrImageTransp);
		// 
	tab->SetCursorRef(m_hpCurTabRef);
		// 
	tab->SetFontNormalRef( o.GetFontNormal() );
	tab->SetFontSelectRef( o.GetFontSelect() );
		// 
	tab->SetTabsScrollingStep( m_TabState.GetTabsScrollingStep() );
		// 
	tab->ShowBorder( m_TabState.IsBorderVisible() );
	tab->EqualTabsSize( m_TabState.IsEqualTabsSize() );
	tab->EnableTabRemove( m_TabState.IsTabRemoveEnable() );
	tab->HideSingleTab( m_TabState.IsHideSingleTab() );
	tab->ShowButtonClose( m_TabState.IsButtonCloseVisible() );
	tab->ShowButtonMenu( m_TabState.IsButtonMenuVisible() );
	tab->ShowButtonsScroll( m_TabState.IsButtonsScrollVisible() );
	tab->WatchCtrlActivity( m_TabState.IsWatchCtrlActivity() );
		// 
	tab->EnableToolTip( m_TabState.IsToolTipEnable() );
	tab->SetButtonCloseToolTipText( m_TabState.GetButtonCloseToolTipText() );
	tab->SetButtonMenuToolTipText( m_TabState.GetButtonMenuToolTipText() );
	tab->SetButtonScrollLeftToolTipText( m_TabState.GetButtonScrollLeftToolTipText() );
	tab->SetButtonScrollRightToolTipText( m_TabState.GetButtonScrollRightToolTipText() );
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::InstallStyle(IStyle *style)
{	assert(style);
		// 
	p.m_pTabStyle = style->GetTabCtrlStyle();
		// 
	SetDrawManager( style->GetDrawManager() );
	SetRecalcManager( style->GetRecalcManager() );
		// 
	if( GetSafeHwnd() )
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
			{	TabCtrl *tab = GetTabCtrl(h);
				p.m_pTabStyle->Install(tab);
			}
		// 
	p.m_DockMarkMngr.SetDrawManager( style->GetDockMarkersDrawManager() );
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
int MultiPaneCtrl::GetBorderWidth() const
{	return p.m_pRecalcManager->GetBorderWidth(this,&p);
}
// 
CSize MultiPaneCtrl::GetSplitterSize() const
{	return p.m_pRecalcManager->GetSplitterSize(this,&p);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
BOOL MultiPaneCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{	*pResult = CWnd::GetParent()->SendMessage(WM_NOTIFY,wParam,lParam);
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{	if(p.m_hSplitDragPane==nullptr)
	{	p.m_hSplitDragPane = p.HitTestSplitter(&point);
		if( !IsSplitterActive(p.m_hSplitDragPane) ) 
			p.m_hSplitDragPane = nullptr;
			// 
		if(p.m_hSplitDragPane)
		{	p.StartSplitterDragging(point);
				// 
			if(p.m_SplitterDragMode==SplitterDraggingStatic)   // primary drawing of drag rect.
			{	CRect rc = GetSplitterRect(p.m_hSplitDragPane);
				const bool horz = p.IsSplitterHorizontal(p.m_hSplitDragPane);
				p.DrawSplitterDragRect(&rc,horz);
			}
		}
	}
		// 
	CWnd::OnLButtonDown(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{	HPANE pane = p.HitTestSplitter(&point);
		// 
	if(pane && IsSplitterActive(pane))
		if( p.CanSplitterDrag(pane) )
		{	const bool horz = p.IsSplitterHorizontal(pane);
			HPANE next = GetNextChildPane(pane);
				// 
			(horz ? p.HalveHeightPanes(pane,next) : p.HalveWidthPanes(pane,next));
			p.UpdateAdjacentPanes(pane,next);   // update two panes around splitter.
		}
		// 
	CWnd::OnLButtonDblClk(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnMButtonDown(UINT nFlags, CPoint point)
{	p.StopSplitterDragging(true);
		// 
	CWnd::OnMButtonDown(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{	p.StopSplitterDragging(true);
		// 
	CWnd::OnRButtonDown(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnMouseMove(UINT nFlags, CPoint point)
{	if(p.m_hSplitDragPane)
	{	const bool canDrag = p.CanSplitterDrag(p.m_hSplitDragPane);
		const bool horz = p.IsSplitterHorizontal(p.m_hSplitDragPane);   // orientation of splitter.
		CRect rcSplitter;
			// 
		if(canDrag)
		{	const CPoint ptShift = point - p.m_ptSplitDragStart;
				// 
			if(horz)
				p.SetDraggingHeight(p.m_hSplitDragPane,ptShift.y,&rcSplitter);
			else
				p.SetDraggingWidth(p.m_hSplitDragPane,ptShift.x,&rcSplitter);
		}
		else
			rcSplitter = GetSplitterRect(p.m_hSplitDragPane);
			// 
		if(p.m_SplitterDragMode==SplitterDraggingStatic)
			p.DrawSplitterDragRect(&rcSplitter,horz);
	}
		// 
	CWnd::OnMouseMove(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{	p.StopSplitterDragging(false);
		// 
	CWnd::OnLButtonUp(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnCaptureChanged(CWnd *pWnd)
{	if(pWnd!=this)
		p.StopSplitterDragging(true);
		// 
	CWnd::OnCaptureChanged(pWnd);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::StartSplitterDragging(CPoint point)
{	m_ptSplitDragStart = point;
	SaveAllPanesState();   // save state for all panes.
	o.SetCapture();
		// 
	m_KeyboardHook.Add(this,&Private::OnKeyDown);
}
// 
void MultiPaneCtrl::Private::StopSplitterDragging(bool reset)
{	if(m_hSplitDragPane==nullptr)
		return;
		// 
	m_KeyboardHook.Delete(this);
		// 
	if(m_SplitterDragMode==SplitterDraggingStatic)
	{	const bool horz = IsSplitterHorizontal(m_hSplitDragPane);
		DrawSplitterDragRect(nullptr,horz);   // erase dragging rect.
			// 
		if(!reset)
		{	RestoreAllPanesState();   // set new state from storage.
			UpdateAdjacentPanes(m_hSplitDragPane,o.GetNextChildPane(m_hSplitDragPane));   // update two panes around splitter.
		}
		else   // reset.
			o.Update();
	}
	else if(m_SplitterDragMode==SplitterDraggingDynamic && reset)
	{	RestoreAllPanesState();   // restore state for all panes.
		o.Update();
	}
		// 
	m_hSplitDragPane = nullptr;
	if( GetCapture() )
		::ReleaseCapture();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::SaveAllPanesState()
{	for(HPANE h=o.GetFirstPane(); h; h=o.GetNextPane(h))   // save state for all panes.
	{	Pane *pn = m_Tree.GetData(h);
		pn->store = pn->real;
	}
}
// 
void MultiPaneCtrl::Private::RestoreAllPanesState()
{	for(HPANE h=o.GetFirstPane(); h; h=o.GetNextPane(h))   // restore state for all panes.
	{	Pane *pn = m_Tree.GetData(h);
		pn->real = pn->store;
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::Private::OnKeyDown(UINT keyCode, UINT /*msgFlag*/)
{	if(keyCode==VK_ESCAPE)
		if( o.IsSplitterDragging(nullptr) )
		{	o.CancelDragging();
			return false;
		}
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::DrawSplitterDragRect(CRect const *rect, bool horz)
{	static CRect rcOld(0,0,0,0);
		// 
	if(m_pDrawManager) 
	{	CDC *dc = o.GetDCEx(nullptr, DCX_CACHE | DCX_LOCKWINDOWUPDATE);
		if(dc)
		{	m_pDrawManager->DrawSplitterDragRect(&o,dc,horz,rect,(!rcOld.IsRectNull() ? &rcOld : nullptr));
			o.ReleaseDC(dc);
		}
	}
	rcOld = (rect ? *rect : CRect(0,0,0,0));
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
BOOL MultiPaneCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{	CPoint pt;
	::GetCursorPos(&pt);
	ScreenToClient(&pt);
		// 
	HPANE pane = p.HitTestSplitter(&pt);
	if(pane && IsSplitterActive(pane))
	{	const bool horz = p.IsSplitterHorizontal(pane);
			// 
		if(horz)
		{	if(p.m_hpCurSplitterHorzRef && *p.m_hpCurSplitterHorzRef)
				SetCursor(*p.m_hpCurSplitterHorzRef);
			else
				SetCursor( LoadCursor(nullptr,IDC_SIZENS) );
		}
		else
			if(p.m_hpCurSplitterVertRef && *p.m_hpCurSplitterVertRef)
				SetCursor(*p.m_hpCurSplitterVertRef);
			else
				SetCursor( LoadCursor(nullptr,IDC_SIZEWE) );
			// 
		return TRUE;
	}
		// 
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::Private::HitTestSplitter(CPoint const *pt)
{	for(HPANE h=m_Tree.GetFirst(); h; h=m_Tree.GetNext(h))
	{	Pane *pn = m_Tree.GetData(h);
			// 
		if( pn->real.rcSplitter.PtInRect(*pt) )
			return h;
	}
	return nullptr;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::SetDraggingWidth(HPANE pane, int offset, CRect *rcdrag/*out*/)
{	HPANE next = o.GetNextChildPane(pane);
		// 
	Pane *p1 = m_Tree.GetData(pane);
	Pane *p2 = m_Tree.GetData(next);
		// 
		// 1.
	Pane::State *state1 = (m_SplitterDragMode==SplitterDraggingStatic ? &p1->real : &p1->store);
	Pane::State *state2 = (m_SplitterDragMode==SplitterDraggingStatic ? &p2->real : &p2->store);
		// 
	const int totalcx = state1->rc.Width() + state2->rc.Width();
	const double totalfactor = state1->factor + state2->factor;
		// 
	int cx1 = state1->rc.Width() + offset;
	if(cx1<m_szMinSize.cx) cx1 = m_szMinSize.cx;
		// 
	int cx2 = totalcx - cx1;
	if(cx2<m_szMinSize.cx)
	{	cx2 = m_szMinSize.cx;
		cx1 = totalcx - cx2;
	}
		// 
		// 2.
	state1 = (m_SplitterDragMode==SplitterDraggingStatic ? &p1->store : &p1->real);
	state2 = (m_SplitterDragMode==SplitterDraggingStatic ? &p2->store : &p2->real);
		// 
	state1->factor = totalfactor * static_cast<double>(cx1)/static_cast<double>(totalcx);
	state2->factor = totalfactor - state1->factor;
	if(state2->factor<0.0) state2->factor = 0.0;
		// 
	state1->rc.right = state1->rc.left + cx1;
	state2->rc.left = state2->rc.right - cx2;
	state1->rcSplitter.SetRect(state1->rc.right,state1->rc.top,state1->rc.right+o.GetSplitterSize().cx,state1->rc.bottom);
		// 
	if(m_SplitterDragMode==SplitterDraggingStatic)
		*rcdrag = state1->rcSplitter;
	else
		UpdateAdjacentPanes(pane,next);   // update two panes around splitter.
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::SetDraggingHeight(HPANE pane, int offset, CRect *rcdrag/*out*/)
{	HPANE next = o.GetNextChildPane(pane);
		// 
	Pane *p1 = m_Tree.GetData(pane);
	Pane *p2 = m_Tree.GetData(next);
		// 
		// 1.
	Pane::State *state1 = (m_SplitterDragMode==SplitterDraggingStatic ? &p1->real : &p1->store);
	Pane::State *state2 = (m_SplitterDragMode==SplitterDraggingStatic ? &p2->real : &p2->store);
		// 
	const int totalcy = state1->rc.Height() + state2->rc.Height();
	const double totalfactor = state1->factor + state2->factor;
		// 
	int cy1 = state1->rc.Height() + offset;
	if(cy1<m_szMinSize.cy) cy1 = m_szMinSize.cy;
		// 
	int cy2 = totalcy - cy1;
	if(cy2<m_szMinSize.cy)
	{	cy2 = m_szMinSize.cy;
		cy1 = totalcy - cy2;
	}
		// 
		// 2.
	state1 = (m_SplitterDragMode==SplitterDraggingStatic ? &p1->store : &p1->real);
	state2 = (m_SplitterDragMode==SplitterDraggingStatic ? &p2->store : &p2->real);
		// 
	state1->factor = totalfactor * static_cast<double>(cy1)/static_cast<double>(totalcy);
	state2->factor = totalfactor - state1->factor;
	if(state2->factor<0.0) state2->factor = 0.0;
		// 
	state1->rc.bottom = state1->rc.top + cy1;
	state2->rc.top = state2->rc.bottom - cy2;
	state1->rcSplitter.SetRect(state1->rc.left,state1->rc.bottom,state1->rc.right,state1->rc.bottom+o.GetSplitterSize().cy);
		// 
	if(m_SplitterDragMode==SplitterDraggingStatic)
		*rcdrag = state1->rcSplitter;
	else
		UpdateAdjacentPanes(pane,next);   // update two panes around splitter.
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::UpdateAdjacentPanes(HPANE pane1, HPANE pane2)
{	CRect *rc1 = &m_Tree.GetData(pane1)->real.rc;
	CRect *rc2 = &m_Tree.GetData(pane2)->real.rc;
		// 
	DeferWindow dw;
	Recalc(&dw,pane1,*rc1,false);
	Recalc(&dw,pane2,*rc2,false);
	dw.Process();
		// 
	CRect rc(rc1->TopLeft(),rc2->BottomRight());
	::RedrawWindow(o.m_hWnd,&rc,nullptr,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::HalveWidthPanes(HPANE prev, HPANE next)
{	Pane *p1 = m_Tree.GetData(prev);
	Pane *p2 = m_Tree.GetData(next);
		// 
	const int totalcx = p1->real.rc.Width() + p2->real.rc.Width();
	const double totalfactor = p1->real.factor + p2->real.factor;
		// 
	int cx1 = totalcx / 2;
	if(cx1<m_szMinSize.cx) cx1 = m_szMinSize.cx;
		// 
	int cx2 = totalcx - cx1;
	if(cx2<m_szMinSize.cx)
	{	cx2 = m_szMinSize.cx;
		cx1 = totalcx - cx2;
	}
		// 
	p1->real.factor = totalfactor * static_cast<double>(cx1)/static_cast<double>(totalcx);
	p2->real.factor = totalfactor - p1->real.factor;
	if(p2->real.factor<0.0) p2->real.factor = 0.0;
		// 
	p1->real.rc.right = p1->real.rc.left + cx1;
	p2->real.rc.left = p2->real.rc.right - cx2;
	p1->real.rcSplitter.SetRect(p1->real.rc.right,p1->real.rc.top,p1->real.rc.right+o.GetSplitterSize().cx,p1->real.rc.bottom);
}
// 
void MultiPaneCtrl::Private::HalveHeightPanes(HPANE prev, HPANE next)
{	Pane *p1 = m_Tree.GetData(prev);
	Pane *p2 = m_Tree.GetData(next);
		// 
	const int totalcy = p1->real.rc.Height() + p2->real.rc.Height();
	const double totalfactor = p1->real.factor + p2->real.factor;
		// 
	int cy1 = totalcy / 2;
	if(cy1<m_szMinSize.cy) cy1 = m_szMinSize.cy;
		// 
	int cy2 = totalcy - cy1;
	if(cy2<m_szMinSize.cy)
	{	cy2 = m_szMinSize.cy;
		cy1 = totalcy - cy2;
	}
		// 
	p1->real.factor = totalfactor * static_cast<double>(cy1)/static_cast<double>(totalcy);
	p2->real.factor = totalfactor - p1->real.factor;
	if(p2->real.factor<0.0) p2->real.factor = 0.0;
		// 
	p1->real.rc.bottom = p1->real.rc.top + cy1;
	p2->real.rc.top = p2->real.rc.bottom - cy2;
	p1->real.rcSplitter.SetRect(p1->real.rc.left,p1->real.rc.bottom,p1->real.rc.right,p1->real.rc.bottom+o.GetSplitterSize().cy);
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::Private::IsSplitterHorizontal(HPANE pane) const
{	assert( o.IsPaneExist(pane) );
	assert( o.GetParentPane(pane) );
		// 
	return !o.IsLineHorizontal( o.GetParentPane(pane) );
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::Private::CanSplitterDrag(HPANE pane)
{	const bool horz = IsSplitterHorizontal(pane);   // orientation of splitter.
		// 
	HPANE parent = o.GetParentPane(pane);
	const CRect rcParent = o.GetPaneRect(parent);
	const int count = o.GetNumberPanes(parent);
		// 
	if(horz)
	{	const int height = count*m_szMinSize.cy + (count-1)*o.GetSplitterSize().cy;
		return (rcParent.Height() > height);
	}
	const int width = count*m_szMinSize.cx + (count-1)*o.GetSplitterSize().cx;
	return (rcParent.Width() > width);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::Private::CanShowButtonClose(TabCtrl const *ctrl)
{	return m_pAbilityManager->CanShowButtonClose(&o,ctrl);
}
// 
bool MultiPaneCtrl::Private::CanShowButtonMenu(TabCtrl const *ctrl)
{	return m_pAbilityManager->CanShowButtonMenu(&o,ctrl);
}
// 
bool MultiPaneCtrl::Private::CanShowButtonScroll(TabCtrl const *ctrl)
{	return m_pAbilityManager->CanShowButtonScroll(&o,ctrl);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::OnTabPreCreate(TabCtrl const *ctrl, HWND hWnd, TCHAR const *text, int image)
{
#ifdef _DEBUG
	if(!m_bDragFinishing) 
		assert(o.GetTabWithWindowID(::GetDlgCtrlID(hWnd),nullptr)==nullptr);   // !!! WINDOW FOR EACH TAB SHOULD HAS UNIQUE ID.
#endif
		// 
	if(m_pNotifyManager)
		m_pNotifyManager->OnTabPreCreate(&o,ctrl,hWnd,text,image);
}
// 
void MultiPaneCtrl::Private::OnTabPostCreate(TabCtrl *ctrl, TabCtrl::HTAB hTab)
{	if(m_pNotifyManager)
		m_pNotifyManager->OnTabPostCreate(&o,ctrl,hTab);
}
// 
void MultiPaneCtrl::Private::OnTabPreDestroy(TabCtrl const *ctrl, TabCtrl::HTAB hTab)
{	if(m_pNotifyManager)
		m_pNotifyManager->OnTabPreDestroy(&o,ctrl,hTab);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::OnButtonCloseClicked(TabCtrl *ctrl, CRect const *rect, CPoint ptScr)
{	if(m_pNotifyManager)
		m_pNotifyManager->OnTabButtonCloseClicked(&o,ctrl,rect,ptScr);
}
// 
void MultiPaneCtrl::Private::OnButtonMenuClicked(TabCtrl *ctrl, CRect const *rect, CPoint ptScr)
{	if(m_pNotifyManager)
		m_pNotifyManager->OnTabButtonMenuClicked(&o,ctrl,rect,ptScr);
}
// 
void MultiPaneCtrl::Private::OnTabSelected(TabCtrl *ctrl, TabCtrl::HTAB hTab)
{	if(m_pNotifyManager)
		m_pNotifyManager->OnTabSelected(&o,ctrl,hTab);
}
// 
void MultiPaneCtrl::Private::OnLButtonDown(TabCtrl const *ctrl, TabCtrl::HTAB hTab, CPoint ptScr)
{	if(m_pNotifyManager)
		m_pNotifyManager->OnTabLButtonDown(&o,ctrl,hTab,ptScr);
}
// 
void MultiPaneCtrl::Private::OnLButtonDblClk(TabCtrl *ctrl, TabCtrl::HTAB hTab, CPoint ptScr)
{	if(m_pNotifyManager)
		m_pNotifyManager->OnTabLButtonDblClk(&o,ctrl,hTab,ptScr);
}
// 
void MultiPaneCtrl::Private::OnRButtonDown(TabCtrl *ctrl, TabCtrl::HTAB hTab, CPoint ptScr)
{	if(m_pNotifyManager)
		m_pNotifyManager->OnTabRButtonDown(&o,ctrl,hTab,ptScr);
}
// 
void MultiPaneCtrl::Private::OnRButtonUp(TabCtrl * ctrl, TabCtrl::HTAB hTab, CPoint ptScr)
{	if(m_pNotifyManager)
		m_pNotifyManager->OnTabRButtonUp(&o,ctrl,hTab,ptScr);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::OnStartDrag(TabCtrl const *ctrl, TabCtrl::HTAB hTab, CPoint ptScr)
{	if(m_pNotifyManager)
		m_pNotifyManager->OnTabStartDrag(&o,ctrl,hTab,ptScr);
}
// 
void MultiPaneCtrl::Private::OnDrag(TabCtrl *pTabSrc, TabCtrl::HTAB hTabSrc, CPoint ptScr, bool outside)
{	if( o.IsTabDragEnable() )
	{		// 
			// show cursor.
		if(m_hpCurDragEnableRef || m_hpCurDragDisableRef)
		{	CRect rc;
			o.GetWindowRect(&rc);
			const bool inside = (rc.PtInRect(ptScr)!=0);
			if(m_hpCurDragEnableRef && *m_hpCurDragEnableRef && inside)
				::SetCursor(*m_hpCurDragEnableRef);
			if(m_hpCurDragDisableRef && *m_hpCurDragDisableRef && !inside)
				::SetCursor(*m_hpCurDragDisableRef);
		}
			// 
			// show markers.
		HPANE paneDst = o.GetDragDstPane(hTabSrc,ptScr);
			// 
		if(paneDst)   // need to show markers.
		{	TabCtrl *pTabDst = o.GetTabCtrl(paneDst);
				// 
			CRect rcTabDst;
			pTabDst->GetClientRect(&rcTabDst);
			pTabDst->ClientToScreen(&rcTabDst);
				// 
			m_hDraggingTab = hTabSrc;
				// 
			if(paneDst!=m_hMarkedPane)
			{	m_hMarkedPane = paneDst;
				if( m_DockMarkMngr.IsCreated() )
					m_DockMarkMngr.ShowMarkers(&rcTabDst);
				m_SelectedMarker = DockingMarkers::PositionNone;
				m_DockMarkMngr.HideInsertionArea(&o);
			}
				// 
			const DockingMarkers::Position selectedMarker = RelayMouseMove(pTabDst,ptScr);   // get marker under cursor.
				// 
				// show select bar.
			if(selectedMarker!=m_SelectedMarker)
			{	if(selectedMarker==DockingMarkers::PositionNone || !m_DockMarkMngr.IsCreated())
					m_DockMarkMngr.HideInsertionArea(&o);   // hide old markers or InsertionFrame.
					// 
				switch(selectedMarker)
				{	case DockingMarkers::PositionLeft: 
						rcTabDst.right = rcTabDst.CenterPoint().x;
						m_DockMarkMngr.ShowInsertionArea(&o,&rcTabDst,false,false,0);
						break;
					case DockingMarkers::PositionTop:
						rcTabDst.bottom = rcTabDst.CenterPoint().y;
						m_DockMarkMngr.ShowInsertionArea(&o,&rcTabDst,false,false,0);
						break;
					case DockingMarkers::PositionRight:
						rcTabDst.left = rcTabDst.CenterPoint().x;
						m_DockMarkMngr.ShowInsertionArea(&o,&rcTabDst,false,false,0);
						break;
					case DockingMarkers::PositionBottom:
						rcTabDst.top = rcTabDst.CenterPoint().y;
						m_DockMarkMngr.ShowInsertionArea(&o,&rcTabDst,false,false,0);
						break;
					case DockingMarkers::PositionCenter:
						m_DockMarkMngr.ShowInsertionArea(&o,&rcTabDst,true,(pTabDst->GetLayout()==TabCtrl::LayoutTop), pTabDst->CalcCtrlAreaHeight() );
						break;
				}
			}
				// 
			m_SelectedMarker = selectedMarker;
		}
		else   // need to hide markers.
			if(m_hMarkedPane)   // there are still visible markers.
			{	if( m_DockMarkMngr.IsCreated() )
					m_DockMarkMngr.HideMarkers();
				m_DockMarkMngr.HideInsertionArea(&o);
				m_hMarkedPane = nullptr;
				m_SelectedMarker = DockingMarkers::PositionNone;
			}
	}
		// 
	if(m_pNotifyManager)
		m_pNotifyManager->OnTabDrag(&o,pTabSrc,hTabSrc,ptScr,outside);
}
// 
MultiPaneCtrl::HPANE MultiPaneCtrl::GetDragDstPane(TabCtrl::HTAB hTabSrc, CPoint ptScr)
{	assert( GetSafeHwnd() );
	assert(hTabSrc==nullptr || GetPaneWithTab(hTabSrc));
		// 
	HPANE hPaneDst = HitTest(ptScr);
	if(hPaneDst==nullptr)
		return nullptr;
		// 
	TabCtrl *pTabDst = GetTabCtrl(hPaneDst);
		// 
	if(hTabSrc)
		if(GetPaneWithTab(hTabSrc)==hPaneDst)
		{	if(pTabDst->GetNumberTabs()==1)   // drag to itself.
				return nullptr;
				// 
			CRect rc(pTabDst->GetWindowsArea());
			pTabDst->ClientToScreen(&rc);
			if(rc.PtInRect(ptScr)==FALSE)
				return nullptr;
		}
		// 
	CRect rcTabDst;
	pTabDst->GetClientRect(&rcTabDst);
	if(rcTabDst.Width()<p.m_szMinSizeForDrop.cx || rcTabDst.Height()<p.m_szMinSizeForDrop.cy)
		return nullptr;
		// 
	return hPaneDst;
}
// 
DockingMarkers::Position MultiPaneCtrl::Private::RelayMouseMove(TabCtrl *pTabDst, CPoint ptScr)
{	DockingMarkers::Position marker = DockingMarkers::PositionNone;
		// 
	if( m_DockMarkMngr.IsCreated() )   // use markers.
		marker = m_DockMarkMngr.RelayMouseMove(ptScr);
	else   // markers wasn't created - use insertion frame.
	{	CRect rc( pTabDst->GetWindowsArea() );
		pTabDst->ClientToScreen(&rc);
			// 
		if( rc.PtInRect(ptScr) )   // inside of child window area of destination TabCtrl.
		{	CRect rcTab(rc);
			rcTab.DeflateRect(rcTab.Width()/3,rcTab.Height()/3);
				// 
			if(rcTab.PtInRect(ptScr) && IsMarkerEnable(nullptr,DockingMarkers::PositionCenter))
				marker = DockingMarkers::PositionCenter;
			else
			{	const CPoint center = rc.CenterPoint();
				if(PtInTriangle(ptScr,CPoint(rc.left,rc.bottom),rc.TopLeft(),center) && IsMarkerEnable(nullptr,DockingMarkers::PositionLeft))
					marker = DockingMarkers::PositionLeft;
				else if(PtInTriangle(ptScr,rc.TopLeft(),CPoint(rc.right,rc.top),center) && IsMarkerEnable(nullptr,DockingMarkers::PositionTop)) 
					marker = DockingMarkers::PositionTop;
				else if(PtInTriangle(ptScr,CPoint(rc.right,rc.top),rc.BottomRight(),center) && IsMarkerEnable(nullptr,DockingMarkers::PositionRight)) 
					marker = DockingMarkers::PositionRight;
				else if(PtInTriangle(ptScr,rc.BottomRight(),CPoint(rc.left,rc.bottom),center) && IsMarkerEnable(nullptr,DockingMarkers::PositionBottom)) 
					marker = DockingMarkers::PositionBottom;
			}
		}
	}
		// 
	if(marker==DockingMarkers::PositionNone)   // check for location inside of control area of destination TabCtrl.
	{	CRect rc( pTabDst->GetCtrlArea() );
		pTabDst->ClientToScreen(&rc);
			// 
		if(rc.PtInRect(ptScr) && IsMarkerEnable(nullptr,DockingMarkers::PositionCenter))
			marker = DockingMarkers::PositionCenter;
	}
		// 
	return marker;
}
// 
void MultiPaneCtrl::Private::OnFinishDrag(TabCtrl *pTabSrc, TabCtrl::HTAB hTabSrc, bool cancel)
{	if( o.IsTabDragEnable() )
	{	m_bDragFinishing = true;
			// 
			// hide markers and select bar.
		if( m_DockMarkMngr.IsCreated() )
			m_DockMarkMngr.HideMarkers();
		m_DockMarkMngr.HideInsertionArea(&o);
			// 
			// insert new and delete old tab.
		if(!cancel && m_SelectedMarker!=DockingMarkers::PositionNone)
		{	if(m_SelectedMarker==DockingMarkers::PositionCenter)
			{	HPANE hPaneDst = m_hMarkedPane;
				TabCtrl *pTabDst = o.GetTabCtrl(hPaneDst);
					// 
				TabCtrl::HTAB hTabDst = nullptr;
				try
				{	if(pTabDst->GetNumberTabs()>0)   // insert tab in 0 position.
					{	hTabDst = o.InsertTab(hPaneDst,pTabDst->GetTabHandleByIndex(0),pTabSrc->GetTabWindow(hTabSrc),_T(""),-1);
						pTabDst->EnsureTabVisible(hTabDst);
					}
					else   // add first tab.
						hTabDst = o.AddTab(hPaneDst,pTabSrc->GetTabWindow(hTabSrc),_T(""),-1);
					pTabDst->CopyTabContent(hTabDst,pTabSrc,hTabSrc);   // copy: text, image, data, tooltip text.
					pTabDst->SelectTab(hTabDst);
						// 
					HPANE hPaneSrc = o.GetPaneWithTabCtrl(pTabSrc);
					DeleteSrcTab(hPaneSrc,pTabSrc,hTabSrc,m_hMarkedPane,m_SelectedMarker);
						// 
					pTabDst->SetFocus();
				}
				catch(std::bad_alloc &)
				{	if(hTabDst)
					{	::SetParent(pTabDst->GetTabWindow(hTabDst),pTabSrc->m_hWnd);   // restore parent window.
						pTabDst->DeleteTab(hTabDst);
					}
				}
			}
			else
			{	HPANE hParent = o.GetParentPane(m_hMarkedPane);
				HWND hWnd = pTabSrc->GetTabWindow(hTabSrc);
					// 
				HPANE hPaneDst = nullptr;
				TabCtrl::HTAB hTabDst = nullptr;
				TabCtrl *pTabDst = nullptr;
					// 
				const bool bNeedHorizOrient = (m_SelectedMarker==DockingMarkers::PositionLeft || m_SelectedMarker==DockingMarkers::PositionRight);
				const bool bInsertPrev = (m_SelectedMarker==DockingMarkers::PositionLeft || m_SelectedMarker==DockingMarkers::PositionTop);
					// 
				try
				{	if(hParent && o.IsLineHorizontal(hParent)==bNeedHorizOrient)
					{	if(bInsertPrev)
							hPaneDst = o.InsertPane(m_hMarkedPane,SpaceNext);
						else
						{	HPANE hNext = o.GetNextChildPane(m_hMarkedPane);
							hPaneDst = (hNext ? o.InsertPane(hNext,SpacePrevious) : o.AddPane(hParent));
						}
					}
					else
					{	o.ConvertPaneToLine(m_hMarkedPane,bNeedHorizOrient);
						if(bInsertPrev)
							hPaneDst = o.InsertPane( o.GetPaneHandleByIndex(m_hMarkedPane,0), SpaceBetween);
						else
							hPaneDst = o.AddPane(m_hMarkedPane);
					}
					pTabDst = o.GetTabCtrl(hPaneDst);
						// 
					hTabDst = o.AddTab(hPaneDst,hWnd,_T(""),-1);   // add first tab.
					pTabDst->CopyTabContent(hTabDst,pTabSrc,hTabSrc);   // copy: text, image, data, tooltip text.
						// 
					HPANE hPaneSrc = o.GetPaneWithTabCtrl(pTabSrc);
					DeleteSrcTab(hPaneSrc,pTabSrc,hTabSrc,m_hMarkedPane,m_SelectedMarker);
						// 
					pTabDst->SetFocus();
				}
				catch(std::bad_alloc &)
				{	if(hTabDst)
					{	::SetParent(hWnd,pTabSrc->m_hWnd);
						pTabDst->DeleteTab(hTabDst);
					}
					if(hPaneDst)
						o.DeletePane(hPaneDst,SpaceBetween);
				}
			}
			o.Update();
		}
			// 
		m_hMarkedPane = nullptr;
		m_SelectedMarker = DockingMarkers::PositionNone;
			// 
		m_bDragFinishing = false;
	}
		// 
	if(m_pNotifyManager)
		m_pNotifyManager->OnTabFinishDrag(&o,pTabSrc,hTabSrc,cancel);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::DeleteSrcTab(HPANE paneSrc, TabCtrl *pTabSrc, TabCtrl::HTAB hTabSrc, HPANE paneMarked, DockingMarkers::Position markerSelected)
{	pTabSrc->DeleteTab(hTabSrc);
		// 
	if(pTabSrc->GetNumberTabs()==0)   // deleted last tab.
	{	Space space = SpaceBetween;
			// 
		HPANE hPrev = o.GetPrevChildPane(paneMarked);
		HPANE hNext = o.GetNextChildPane(paneMarked);
			// 
		if(o.GetParentPane(paneSrc)==o.GetParentPane(paneMarked))
			if(hPrev==paneSrc ||
				(markerSelected==DockingMarkers::PositionLeft && hPrev && o.GetPrevChildPane(hPrev)==paneSrc))
				space = SpacePrevious;
			else if(hNext==paneSrc || 
				(markerSelected==DockingMarkers::PositionRight && hNext && o.GetNextChildPane(hNext)==paneSrc))
				space = SpaceNext;
			// 
			// delete pane down with optimization.
		DeleteOptimizDown(paneSrc, o.IsDropOptimizationEnable(), space);
	}
	else   // there are still some tabs.
		pTabSrc->SetFocus();   // for stable work selection between TabCtrls.
}
// 
void MultiPaneCtrl::Private::DeleteOptimizDown(HPANE paneSrc, bool optimize, Space space)
{	HPANE parentPane = nullptr;
		// 
		// delete pane down (in the direction of root).
	for(HPANE h=paneSrc; h && o.GetNumberPanes(h)==0; h=parentPane)
	{	parentPane = o.GetParentPane(h);
		if(parentPane)
			o.DeletePane(h,space);
	}
		// 
	if(optimize)   // use optimization of layout.
		for(; parentPane; )
		{	const int count = o.GetNumberPanes(parentPane);
				// 
			if(count==0)   // has tabs.
			{	HPANE parent = o.GetParentPane(parentPane);
					// 
				if(parent)
					if(o.GetNumberPanes(parent)==1 || o.GetTabCtrl(parentPane)->GetNumberTabs()==0)
						o.DeletePane(parentPane,SpaceBetween);
					else
						break;
				parentPane = parent;
			}
			else if(count==1)   // has one child pane.
				parentPane = o.GetPaneHandleByIndex(parentPane,0);
			else   // has more than one child pane.
			{	HPANE parent = o.GetParentPane(parentPane);
					// 
				if(parent && o.GetNumberPanes(parent)==1)
				{	o.DeleteIntermediatePane(parentPane);
					parentPane = parent;
				}
				else
					break;
			}
		}
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::DeleteOptimizDown(HPANE hPane)
{	assert( GetSafeHwnd() );
	assert( IsPaneExist(hPane) );
	assert( GetParentPane(hPane) );   // you cannot delete the root pane.
	assert(GetNumberPanes(hPane)==0);   // source pane mustn't has child panes.
	assert(GetTabCtrl(hPane)->GetNumberTabs()==0);   // source tab control mustn't has child tabs.
		// 
	p.DeleteOptimizDown(hPane,true,SpaceBetween);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::Private::IsMarkerEnable(DockingMarkers * /*pObj*/, DockingMarkers::Position dockingSide)
{	if(dockingSide==DockingMarkers::PositionCenter && 
		o.GetTabCtrl(m_hMarkedPane)->IsTabExist(m_hDraggingTab))
		return false;
		// 
	return m_pAbilityManager->CanDrop(&o,m_hDraggingTab,dockingSide,m_hMarkedPane);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
int MultiPaneCtrl::Private::GetBorderWidth(MultiPaneCtrl const * /*ctrl*/, IRecalc * /*base*/)
{	return 1;
}
// 
CSize MultiPaneCtrl::Private::GetSplitterSize(MultiPaneCtrl const * /*ctrl*/, IRecalc * /*base*/)
{	return CSize(4,4);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::Tabs::Tabs()
{	Owner = nullptr;
}
// 
MultiPaneCtrl::Tabs::~Tabs()
{	if(Owner)
		delete static_cast<Private::TabsData*>(Owner);
}
// 
int MultiPaneCtrl::Tabs::Add(HWND wnd, TCHAR const *text, int image)
{	return Add(wnd,text,image,_T(""),false,0);
}
// 
int MultiPaneCtrl::Tabs::Add(HWND wnd, TCHAR const *text, int image, TCHAR const *tooltip, bool disable, __int64 data)
{	assert(wnd && ::IsWindow(wnd));
	assert( ::GetDlgCtrlID(wnd) );
	assert(text && tooltip);
	assert(image>=-1);
		// 
	if(!Owner)
		Owner = static_cast<PSID>(new Private::TabsData);
	Private::TabsData *cont = static_cast<Private::TabsData*>(Owner);
		// 
	int id = ::GetDlgCtrlID(wnd);
	assert(Private::TabsData::GetTabWithCtrlID(this,id)==nullptr);   // window with the same id must not be added twice.
		// 
	typedef std::map<int,Private::TabImpl>::iterator i_map;
	std::pair<i_map,bool> pr = cont->id_to_tab.insert( std::pair<int,Private::TabImpl>(id,Private::TabImpl()) );
	Private::TabImpl *ptd = &pr.first->second;
	cont->tabs.push_back(ptd);
		// 
	ptd->wnd = wnd;
	ptd->text = text;
	ptd->image = image;
	ptd->tooltip = tooltip;
	ptd->disable = disable;
	ptd->data = data;
		// 
	return static_cast<int>( cont->tabs.size() )-1;
}
//
void MultiPaneCtrl::Tabs::DeleteAll()
{	if(Owner)
	{	Private::TabsData *ptr = static_cast<Private::TabsData*>(Owner);
		ptr->tabs.clear();
		ptr->id_to_tab.clear();
	}
}
// 
MultiPaneCtrl::ITab const *MultiPaneCtrl::Tabs::operator[](int idx) const
{	if(!Owner)
		return nullptr;
		// 
	Private::TabsData *cont = static_cast<Private::TabsData*>(Owner);
	assert(idx>=0 && idx<static_cast<int>(cont->tabs.size()));
		// 
	return cont->tabs[idx];
}
// 
int MultiPaneCtrl::Tabs::GetNumber() const
{	if(!Owner)
		return 0;
		// 
	Private::TabsData *cont = static_cast<Private::TabsData*>(Owner);
	return static_cast<int>( cont->tabs.size() );
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::GetAllTabs(Tabs *tabs/*out*/) const
{	try
	{	tabs->DeleteAll();
			// 
		for(HPANE h=GetFirstPane(); h; h=GetNextPane(h))
			if(GetNumberPanes(h)==0)
			{	TabCtrl *tabCtrl = GetTabCtrl(h);
					// 
				for(int i=0, c=tabCtrl->GetNumberTabs(); i<c; ++i)
				{	TabCtrl::HTAB tab = tabCtrl->GetTabHandleByIndex(i);
						// 
					tabs->Add(tabCtrl->GetTabWindow(tab),tabCtrl->GetTabText(tab),tabCtrl->GetTabImage(tab),
						tabCtrl->GetTabTooltipText(tab),tabCtrl->IsTabDisabled(tab),tabCtrl->GetTabData(tab));
				}
			}
	}
	catch(std::bad_alloc &)
	{	tabs->DeleteAll();
		return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SaveState(CWinApp *app, TCHAR const *section, TCHAR const *entry) const
{	assert(app && section && entry);
		// 
	if( GetSafeHwnd() )
	{	CMemFile file;
		CArchive ar(&file,CArchive::store);
		if( !SaveState(&ar) )
			return false;
		ar.Flush();
		ar.Close();
			// 
		const UINT dataSize = static_cast<UINT>( file.GetLength() );
		BYTE *data = file.Detach();
		const bool res = app->WriteProfileBinary(section,entry,data,dataSize)!=0;
		free(data);
		return res;
	}
	return false;
}
// 
bool MultiPaneCtrl::LoadState(CWinApp *app, TCHAR const *section, TCHAR const *entry, Tabs const *tabs, bool ignoreLack)
{	assert( GetSafeHwnd() );
	assert(app && section && entry);
		//
	bool res = false;
	BYTE *data = nullptr;
	try
	{	UINT dataSize;
		if( app->GetProfileBinary(section,entry,&data,&dataSize) )
		{	CMemFile file(data,dataSize);
			CArchive ar(&file,CArchive::load);
			res = LoadState(&ar,tabs,ignoreLack);
		}
	}
	catch(CMemoryException* pEx)
	{	pEx->Delete();
	}
	if(data)
		delete [] data;
		// 
	return res;
}
// 
bool MultiPaneCtrl::LoadStateAndUpdate(CWinApp *app, TCHAR const *section, TCHAR const *entry, Tabs const *tabs, bool ignoreLack)
{	SetRedraw(FALSE);
		// 
	const bool res = LoadState(app,section,entry,tabs,ignoreLack);
		// 
	Update();
	SetRedraw(TRUE);
	RedrawWindow(nullptr,nullptr,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		// 
	return res;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SaveState(CArchive *ar) const
{	assert(ar);
		// 
	if( GetSafeHwnd() )
	{	try
		{	p.SaveStateInner(ar);
			return true;
		}
		catch(CMemoryException* pEx)
		{	pEx->Delete();
		}
		catch(CArchiveException* pEx)
		{	pEx->Delete();
		}
		catch(...)
		{
		}
	}
	return false;
}
// 
bool MultiPaneCtrl::LoadState(CArchive *ar, Tabs const *tabs, bool ignoreLack)
{	assert( GetSafeHwnd() );
		// 
	try
	{	if( !p.LoadStateInner(ar,tabs,ignoreLack) )
		{	DeleteAllPanes();
			return false;
		}
		return true;
	}
	catch(CMemoryException* pEx)
	{	pEx->Delete();
	}
	catch(CArchiveException* pEx)
	{	pEx->Delete();
	}
	catch(...)
	{
	}
	return false;
}
// 
bool MultiPaneCtrl::LoadStateAndUpdate(CArchive *ar, Tabs const *tabs, bool ignoreLack)
{	SetRedraw(FALSE);
		// 
	const bool res = LoadState(ar,tabs,ignoreLack);
		// 
	Update();
	SetRedraw(TRUE);
	RedrawWindow(nullptr,nullptr,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		// 
	return res;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Private::SaveStateInner(CArchive *ar) const
{	SaveStateInner(ar,nullptr);
}
// 
void MultiPaneCtrl::Private::SaveStateInner(CArchive *ar, HPANE pane) const
{	int count = o.GetNumberPanes(pane);
	*ar << count;
		// 
	if(count>0)
	{	const char horz = static_cast<char>( o.IsLineHorizontal(pane) );
		*ar << horz;
			// 
		int i;
		for(i=0; i<count; ++i)
		{	HPANE h = o.GetPaneHandleByIndex(pane,i);
			SaveStateInner(ar,h);
		}
		for(i=0; i<count; ++i)   // save factors.
		{	HPANE h = o.GetPaneHandleByIndex(pane,i);
				// 
			const double factor = m_Tree.GetData(h)->real.factor;
			*ar << factor;
		}
	}
	else
	{	TabCtrl *tabCtrl = o.GetTabCtrl(pane);
			// 
		count = tabCtrl->GetNumberTabs();
		*ar << count;
			// 
		if(count>0)
		{	if(count>1)
			{	TabCtrl::HTAB hSelTab = tabCtrl->GetSelectedTab();
				*ar << (hSelTab ? tabCtrl->GetTabIndexByHandle(hSelTab) : -1);
			}
				// 
			for(int i=0, c=count; i<c; ++i)
			{	HWND hWnd = tabCtrl->GetTabWindow( tabCtrl->GetTabHandleByIndex(i) );
					// 
				assert( ::IsWindow(hWnd) );
					// 
				*ar << ::GetDlgCtrlID(hWnd);
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::Private::LoadStateInner(CArchive *ar, Tabs const *tabs, bool ignoreLack)
{	o.DeleteAllPanes();
		// 
	std::list<HPANE> emptyPanes;   // empty panes to delete.
		// 
	try
	{	if( !LoadStateInner(ar,o.GetRootPane(),tabs,ignoreLack,&emptyPanes/*out*/) )
			return false;
			// 
		for(std::list<HPANE>::iterator i=emptyPanes.begin(), e=emptyPanes.end(); i!=e; ++i)
			if(o.IsPaneExist(*i) && o.GetParentPane(*i))
				o.DeleteOptimizDown(*i);
			// 
		return true;
	}
	catch(std::bad_alloc &)
	{	o.DeleteAllPanes();
		throw;
	}
}
// 
bool MultiPaneCtrl::Private::LoadStateInner(CArchive *ar, HPANE parent, Tabs const *tabs, bool ignoreLack, std::list<HPANE> *panes/*out*/)
{	int count;
	*ar >> count;
		// 
	if(count>0)
	{	char horz;
		*ar >> horz;
			// 
		int i;
		for(i=0; i<count; ++i)
		{	HPANE h = (i==0 ? o.ConvertPaneToLine(parent,horz!=0) : o.AddPane(parent));
			if( !LoadStateInner(ar,h,tabs,ignoreLack,panes) )
				return false;
		}
		for(i=0; i<count; ++i)
		{	HPANE h = o.GetPaneHandleByIndex(parent,i);
				// 
			double factor;
			*ar >> factor;
			m_Tree.GetData(h)->real.factor = factor;
		}
	}
	else
	{	*ar >> count;
			// 
		if(count>0)
		{	TabCtrl *tabCtrl = o.GetTabCtrl(parent);
				// 
			int selIdx = -1;
			if(count>1)
				*ar >> selIdx;
				// 
			for(int i=0, c=count; i<c; ++i)
			{	int id;
				*ar >> id;
					// 
				TabImpl const *tabData = TabsData::GetTabWithCtrlID(tabs,id);
					// 
				if(tabData==nullptr)
				{	if(ignoreLack)
					{	if(selIdx!=-1)
							if(selIdx==i)
								selIdx = -1;
							else if(i<selIdx)
								--selIdx;
						continue;
					}
						// 
					TRACE("Not found tab with control id=%d (MultiPaneCtrl::Private::LoadStateInner)",id);
					return false;
				}
					// 
				TabCtrl::HTAB h = o.AddTab(parent,tabData->wnd,tabData->text,tabData->image);
				tabCtrl->SetTabTooltipText(h,tabData->tooltip);
				tabCtrl->DisableTab(h,tabData->disable);
				tabCtrl->SetTabData(h,tabData->data);
			}
				// 
			const int finalCount = tabCtrl->GetNumberTabs();
				// 
			if(selIdx>=0 && selIdx<finalCount)
			{	TabCtrl::HTAB h = tabCtrl->GetTabHandleByIndex(selIdx);
				tabCtrl->SelectTab(h);
			}
				// 
			if(finalCount==0)
				panes->push_back(parent);
		}
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::Private::LoadImage(HMODULE moduleRes/*or null*/, UINT resID, bool pngImage, Gdiplus::Bitmap **bmp/*out*/) const
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
//
bool MultiPaneCtrl::Private::IsDescendantWindow(HWND wndDst, HWND wndSrc) const
{	for(HWND h=wndSrc; h; h=::GetParent(h))
		if(h==wndDst)
			return true;
	return false;
}
/////////////////////////////////////////////////////////////////////////////
//
bool MultiPaneCtrl::Private::PtInTriangle(CPoint const &pt, CPoint const &v1, CPoint const &v2, CPoint const &v3) const
{	POINT pts[3];
	pts[0] = v1; pts[1] = v2; pts[2] = v3;
	CRgn rgn;
	rgn.CreatePolygonRgn(pts,sizeof(pts)/sizeof(POINT),WINDING);
	return rgn.PtInRegion(pt)!=0;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// MultiPaneCtrlDraw.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrlDrawBase::DrawSplitter(MultiPaneCtrl const *ctrl, CDC *dc, bool /*horz*/, CRect const *rect)
{	HBRUSH hBrush = ::CreateSolidBrush( GetSplitterColor(ctrl) );
	::FillRect(dc->m_hDC,rect,hBrush);
	::DeleteObject(hBrush);
}
/////////////////////////////////////////////////////////////////////////////
//
void MultiPaneCtrlDrawBase::DrawSplitterDragRect(MultiPaneCtrl const *ctrl, CDC *dc, bool /*horz*/, CRect const *rectNew, CRect const *rectOld)
{	if(!rectOld)   // first draw.
		DrawDragRect(ctrl,dc,rectNew);
	else if(!rectNew)   // last draw.
		DrawDragRect(ctrl,dc,rectOld);
	else
	{	CRgn rgn, rgnNew, rgnOld;
		rgn.CreateRectRgn(0,0,0,0);
		rgnNew.CreateRectRgnIndirect(rectNew);
		rgnOld.CreateRectRgnIndirect(rectOld);
			// draw new.
		rgn.CombineRgn(&rgnNew,&rgnOld,RGN_DIFF);
		dc->SelectClipRgn(&rgn);
		DrawDragRect(ctrl,dc,rectNew);
			// erase old.
		rgn.CombineRgn(&rgnOld,&rgnNew,RGN_DIFF);
		dc->SelectClipRgn(&rgn);
		DrawDragRect(ctrl,dc,rectOld);
			// 
		dc->SelectClipRgn(nullptr,RGN_COPY);
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrlDrawBase::DrawBorder(MultiPaneCtrl const *ctrl, CDC *dc, CRect const *rect)
{	CPen pen(PS_SOLID,1, GetBorderColor(ctrl) );
	CPen *pOldPen = dc->SelectObject(&pen);
	dc->MoveTo(rect->left,rect->top);
	dc->LineTo(rect->left,rect->bottom-1);
	dc->LineTo(rect->right-1,rect->bottom-1);
	dc->LineTo(rect->right-1,rect->top);
	dc->LineTo(rect->left,rect->top);
	dc->SelectObject(pOldPen);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF MultiPaneCtrlDrawBase::GetSplitterColor(MultiPaneCtrl const * /*ctrl*/)
{	return ::GetSysColor(COLOR_BTNFACE);
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF MultiPaneCtrlDrawBase::GetBorderColor(MultiPaneCtrl const * /*ctrl*/)
{	return ::GetSysColor(COLOR_BTNSHADOW);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrlDrawBase::DrawDragRect(MultiPaneCtrl const * /*ctrl*/, CDC *dc, CRect const *rect)
{	CBrush *pOldBrush = dc->SelectObject( GetDragBrush() );
	dc->PatBlt(rect->left,rect->top,rect->Width(),rect->Height(),PATINVERT);
	dc->SelectObject(pOldBrush);
}
/////////////////////////////////////////////////////////////////////////////
// 
CBrush *MultiPaneCtrlDrawBase::GetDragBrush()
{	return CDC::GetHalftoneBrush();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////












