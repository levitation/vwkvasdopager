#ifndef SAMPLE_H
#define SAMPLE_H

// DeskBand example
DECLARE_MENU_MAP(DeskBandMenu)
extern const CLSID CLSID_SampleDeskBand;

class CSampleDeskBand : public CComCoClass<CSampleDeskBand, &CLSID_SampleDeskBand>,
	public CDeskBand<&CLSID_SampleDeskBand,DeskBandMenu>
{
public:
	typedef CDeskBand<&CLSID_SampleDeskBand,DeskBandMenu> BaseClass;

	CSampleDeskBand();
	~CSampleDeskBand();

	DECLARE_REGISTRY_RESOURCEID(IDR_DESKBAND)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

protected:
	virtual LRESULT OnPaint();
	virtual LRESULT OnCommand( WPARAM, LPARAM );
//	virtual LRESULT OnCreate( LPCREATESTRUCT );
//	virtual LRESULT OnDestroy();
//	virtual LRESULT OnMsg( HWND, UINT, WPARAM, LPARAM );
};

// Vertical Explorer Bar example
/*extern const CLSID CLSID_SampleVerticalBar;

class CSampleVerticalBar : public CComCoClass<CSampleVerticalBar, &CLSID_SampleVerticalBar>,
	public CDeskBand<&CLSID_SampleVerticalBar>
{
public:
	typedef CDeskBand<&CLSID_SampleVerticalBar> BaseClass;

	CSampleVerticalBar();
	~CSampleVerticalBar();

	DECLARE_REGISTRY_RESOURCEID(IDR_VERTICALBAR)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

protected:
	virtual LRESULT OnPaint();
//	virtual LRESULT OnCreate( LPCREATESTRUCT );
//	virtual LRESULT OnDestroy();
//	virtual LRESULT OnCommand( WPARAM, LPARAM );
//	virtual LRESULT OnMsg( HWND, UINT, WPARAM, LPARAM );
};*/

// Horizontal Explorer Bar example
/*extern const CLSID CLSID_SampleHorizontalBar;

class CSampleHorizontalBar : public CComCoClass<CSampleHorizontalBar, &CLSID_SampleHorizontalBar>,
	public CDeskBand<&CLSID_SampleHorizontalBar>
{
public:
	typedef CDeskBand<&CLSID_SampleHorizontalBar> BaseClass;

	CSampleHorizontalBar();
	~CSampleHorizontalBar();

	DECLARE_REGISTRY_RESOURCEID(IDR_HORIZONTALBAR)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

protected:
	virtual LRESULT OnPaint();
//	virtual LRESULT OnCreate( LPCREATESTRUCT );
//	virtual LRESULT OnDestroy();
//	virtual LRESULT OnCommand( WPARAM, LPARAM );
//	virtual LRESULT OnMsg( HWND, UINT, WPARAM, LPARAM );
};*/

#endif // SAMPLE_H
