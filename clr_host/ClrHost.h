#pragma once
#include "StdAfx.h"

_COM_SMARTPTR_TYPEDEF(ISupportErrorInfo, __uuidof(ISupportErrorInfo));
_COM_SMARTPTR_TYPEDEF(IErrorInfo, __uuidof(IErrorInfo));

/// <summary>
///     Interface to the CLR from the unmanaged application
/// </summary>
class ATL_NO_VTABLE CClrHost : public CComObjectRootEx<CComSingleThreadModel>,
	public IHostControl,
	public IHostGCManager,
	public IUnmanagedHost
{
protected:
	BEGIN_COM_MAP(CClrHost)
		COM_INTERFACE_ENTRY(IHostControl)
		COM_INTERFACE_ENTRY(IHostGCManager)
		COM_INTERFACE_ENTRY(IUnmanagedHost)
	END_COM_MAP()

private:
	typedef map<const std::wstring, IManagedHost *>  AppDomainManagerMap;
	typedef map<const std::wstring, ICLRRuntimeHost *>  CLRRunTimeMap;

	bool                    m_started;
	
	ICLRControl             *m_pClrControl;
	AppDomainManagerMap     m_appDomainManagers;
	AppDomainManagerMap     m_NewlyCreatedAppDomains;
	CLRRunTimeMap			m_CLRRuntimeMap;

private:
	static const wchar_t    *AppDomainManagerAssembly;
	static const wchar_t    *AppDomainManagerType;
	std::wstring			m_lastCLR;


public:
	CClrHost();
	~CClrHost();

protected:
	HRESULT FinalConstruct();

public:
	static HRESULT BindToRuntime(IUnmanagedHost **pHost);
	STDMETHODIMP CreateAppDomainForQuery(std::string FnName);

	// IHostControl
public:
	STDMETHODIMP GetHostManager(const IID &riid, void **ppvObject);
	STDMETHODIMP SetAppDomainManager(DWORD dwAppDomainId, IUnknown *pUnkAppDomainManager);

	// IUnmanagedHost
public:
	STDMETHODIMP raw_Start();
	STDMETHODIMP raw_Stop();
	STDMETHODIMP get_DefaultManagedHost(IManagedHost **ppHost);
	STDMETHODIMP raw_GetManagedHost(long appDomain, BSTR clr, IManagedHost **ppHost);

	// IHostGCManager
public:
	STDMETHODIMP SuspensionEnding(DWORD generation);
	STDMETHODIMP SuspensionStarting();
	STDMETHODIMP ThreadIsBlockingForSuspension();
};

// we don't have an AppDomainManager for the specified domain
#define E_NOMANAGEDHOST     MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x201);