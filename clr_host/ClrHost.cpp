
#include "ClrHost.h"

const wchar_t *CClrHost::AppDomainManagerAssembly = L"MySQLHostManager, Version=1.0.0.0, PublicKeyToken=71c4a5d4270bd29c";
const wchar_t *CClrHost::AppDomainManagerType = L"MySQLHostManager.MySQLHostManager";

bool g_CLRHasBeenLoaded = false;


/// <summary>
///     Initialize the host
/// </summary>

class CADMHostModule : public CAtlExeModuleT < CADMHostModule > { };
CADMHostModule _AtlModule;

CClrHost::CClrHost() : m_started(false), m_pClrControl(NULL)
{
	

	return;
}

/// <summary>
///     Clean up the host
/// </summary>
CClrHost::~CClrHost()
{
	//// free the AppDomainManagers
	//for (AppDomainManagerMap::iterator iAdm = m_appDomainManagers.begin(); iAdm != m_appDomainManagers.end(); iAdm++)
	//	iAdm->second->Release();

	//// release the CLR
	//if (m_pClrControl != NULL)
	//	m_pClrControl->Release();
	return;
}

/// <summary>
///     Finish setting up the CLR host
/// </summary>
#pragma warning( disable : 4996 )
HRESULT CClrHost::FinalConstruct()
{
	//load the CLR into the process
	/*return CorBindToRuntimeEx(NULL,
		NULL,
		0,
		CLSID_CLRRuntimeHost,
		IID_ICLRRuntimeHost,
		reinterpret_cast<LPVOID *>(&m_pClr));*/

	//load the CLR into the process
	ICLRMetaHost       *pMetaHost = NULL;
	ICLRMetaHostPolicy *pMetaHostPolicy = NULL;
	ICLRDebugging      *pCLRDebugging = NULL;
	HRESULT hr;
	hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost,
		(LPVOID*)&pMetaHost);
	/*hr = CLRCreateInstance(CLSID_CLRMetaHostPolicy, IID_ICLRMetaHostPolicy,
	(LPVOID*)&pMetaHostPolicy);*/
	/*hr = CLRCreateInstance(CLSID_CLRDebugging, IID_ICLRDebugging,
	(LPVOID*)&pCLRDebugging);*/

	// Enumeration example from COM books.
	IEnumUnknown * pRtEnum = NULL;
	ICLRRuntimeInfo *info = NULL;
	ULONG fetched = 0;
	ICLRRuntimeHost *m_pClr = NULL;
	bool runtimesLoaded = false;
	pMetaHost->EnumerateLoadedRuntimes(GetCurrentProcess(), &pRtEnum);
	WCHAR strName[128];
	DWORD len = 128;

	while ((hr = pRtEnum->Next(1, (IUnknown **)&info, &fetched)) == S_OK && fetched > 0)
	{
		ZeroMemory(strName, sizeof(strName));
		info->GetVersionString(strName, &len);
		hr = info->GetInterface(CLSID_CLRRuntimeHost,
			IID_ICLRRuntimeHost,
			reinterpret_cast<LPVOID *>(&m_pClr));
		if (!SUCCEEDED(hr))
			printf("hr failed....");
		runtimesLoaded = true;
		this->m_lastCLR.assign(strName);
	}
	pRtEnum->Release();
	pRtEnum = NULL;

	if (!runtimesLoaded)
	{
		pMetaHost->EnumerateInstalledRuntimes(&pRtEnum);
		while ((hr = pRtEnum->Next(1, (IUnknown **)&info, &fetched)) == S_OK && fetched > 0)
		{
			ZeroMemory(strName, sizeof(strName));
			
			info->GetVersionString(strName, &len);
			hr = info->GetInterface(CLSID_CLRRuntimeHost,
				IID_ICLRRuntimeHost,
				reinterpret_cast<LPVOID *>(&m_pClr));
			if (!SUCCEEDED(hr))
				printf("hr failed....");
			m_CLRRuntimeMap[std::wstring(strName)] = m_pClr;
			this->m_lastCLR.assign(strName);
		}
		pRtEnum->Release();
	}
	pMetaHost->Release();
	return S_OK;
}



/// <summary>
///     Create a host object, and bind to the CLR
/// </summary>
HRESULT CClrHost::BindToRuntime(__deref_in IUnmanagedHost **pHost)
{
	_ASSERTE(pHost != NULL);
	*pHost = NULL;

	CComObject<CClrHost> *pClrHost = NULL;
	HRESULT hrCreate = CComObject<CClrHost>::CreateInstance(&pClrHost);

	if (SUCCEEDED(hrCreate))
	{
		pClrHost->AddRef();
		*pHost = static_cast<IUnmanagedHost *>(pClrHost);
	}

	return hrCreate;
}

/// <summary>
///     Get a manager from the host
/// </summary>
STDMETHODIMP CClrHost::GetHostManager(const IID &RIID, __deref_opt_out_opt void **ppvObject)
{
	if (ppvObject == NULL)
		return E_POINTER;

	if (RIID == IID_IHostGCManager)
	{
		*ppvObject = (IHostGCManager*)this;
		return S_OK;
	}


	*ppvObject = NULL;
	return E_NOINTERFACE;
}

/// <summary>
///     Register an AppDomain and its AppDomainManager
/// </summary>
/// <param name="dwAppDomainId">ID of the AppDomain being registered</param>
/// <param name="pUnkAppDomainManager">AppDomainManager for the domain</param>
STDMETHODIMP CClrHost::SetAppDomainManager(DWORD dwAppDomainId, __in IUnknown *pUnkAppDomainManager)
{
	// get the managed host interface
	IManagedHost *pAppDomainManager = NULL;
	if (FAILED(pUnkAppDomainManager->QueryInterface(__uuidof(IManagedHost), reinterpret_cast<void **>(&pAppDomainManager))))
	{
		_ASSERTE(!"AppDomainManager does not implement IManagedHost");
		return E_NOINTERFACE;
	}
	// register ourselves as the unmanaged host
	HRESULT hrSetUnmanagedHost = pAppDomainManager->raw_SetUnmanagedHost(static_cast<IUnmanagedHost *>(this));
	if (FAILED(hrSetUnmanagedHost))
		return hrSetUnmanagedHost;

	auto clr = std::wstring(pAppDomainManager->GetCLR());
	// save a copy
	m_appDomainManagers[clr] = pAppDomainManager;
	return S_OK;
}

/// <summary>
///     Start the CLR
/// </summary>
STDMETHODIMP CClrHost::raw_Start()
{
	// we should have bound to the runtime, but not yet started it upon entry
	//_ASSERTE(m_pClr != NULL
	if (!m_started)
	{
		_ASSERTE(!m_started);
		//if (m_pClr == NULL)
		//	return E_FAIL;

		for (auto &x : m_CLRRuntimeMap)
		{

			ICLRRuntimeHost *m_pClr = x.second;
			// get the CLR control object
			HRESULT hrClrControl = m_pClr->GetCLRControl(&m_pClrControl);
			if (FAILED(hrClrControl))
				return hrClrControl;

			// set ourselves up as the host control
			HRESULT hrHostControl = m_pClr->SetHostControl(static_cast<IHostControl *>(this));

			// get the host protection manager
			ICLRHostProtectionManager *pHostProtectionManager = NULL;
			HRESULT hrGetProtectionManager = m_pClrControl->GetCLRManager(
				IID_ICLRHostProtectionManager,
				reinterpret_cast<void **>(&pHostProtectionManager));
			if (FAILED(hrGetProtectionManager))
				return hrGetProtectionManager;

			// setup host proctection
			HRESULT hrHostProtection = pHostProtectionManager->SetProtectedCategories(
				(EApiCategories)(eSynchronization | eSelfAffectingThreading));
			pHostProtectionManager->Release();

			if (FAILED(hrHostProtection))
				return hrHostProtection;


			// setup the AppDomainManager
			HRESULT hrSetAdm = m_pClrControl->SetAppDomainManagerType(AppDomainManagerAssembly, AppDomainManagerType);
			if (FAILED(hrSetAdm))
				return hrSetAdm;


			// finally, start the runtime
			HRESULT hrStart = m_pClr->Start();
			if (FAILED(hrStart))
				return hrStart;
		}

		// mark as started
		m_started = true;
	}
	return S_OK;
}


/// <summary>
///     Stop the CLR
/// </summary>
STDMETHODIMP CClrHost::raw_Stop()
{
	_ASSERTE(m_started);
	// first send a Dispose call to the managed hosts
	for (AppDomainManagerMap::iterator iAdm = m_appDomainManagers.begin(); iAdm != m_appDomainManagers.end(); iAdm++)
		iAdm->second->raw_Dispose();

	// then, shut down the CLR
	return S_OK; // m_pClr->Stop();
}

/// <summary>
///     Get the AppDomainManager for the default AppDomain
/// </summary>
STDMETHODIMP CClrHost::get_DefaultManagedHost(__out IManagedHost **ppHost)
{
	// just get the AppDomainManager for the default AppDomain
	return raw_GetManagedHost(1, BSTR(m_lastCLR.c_str()), ppHost);
}

/// <summary>
///     Get the AppDomainManager for a specific AppDomain
/// </summary>
STDMETHODIMP CClrHost::raw_GetManagedHost(long appDomain, BSTR clr, IManagedHost **ppHost)
{
	_ASSERTE(m_started);

	if (ppHost == NULL)
		return E_POINTER;

	// get the AppDomainManager for the specified domain
	auto iHost = m_appDomainManagers[clr];

	// see if we've got a host
	if (iHost == NULL)
	{
		*ppHost = NULL;
		return E_NOMANAGEDHOST;
	}
	else
	{
		*ppHost = iHost;
		(*ppHost)->AddRef();
		return S_OK;
	}
}

STDMETHODIMP CClrHost::raw_GetSpecificManagedHost(BSTR clr, IManagedHost **ppHost)
{
	_ASSERTE(m_started);

	if (ppHost == NULL)
		return E_POINTER;

	// get the AppDomainManager for the specified domain
	auto iHost = m_NewlyCreatedAppDomains[clr];

	// see if we've got a host
	if (iHost == NULL)
	{
		*ppHost = NULL;
		return E_NOMANAGEDHOST;
	}
	else
	{
		*ppHost = iHost;
		(*ppHost)->AddRef();
		return S_OK;
	}
}


// IHostGCManager
STDMETHODIMP CClrHost::SuspensionEnding(DWORD generation){ return S_OK; }
STDMETHODIMP CClrHost::SuspensionStarting(){ return S_OK; }
STDMETHODIMP CClrHost::ThreadIsBlockingForSuspension(){ return S_OK; }

STDMETHODIMP CClrHost::raw_CreateAppDomainForQuery(BSTR FnName, BSTR *pRetVal)
{
	IManagedHostPtr pAppMgr = this->GetDefaultManagedHost();
	IManagedHostPtr pNewDomain = pAppMgr->CreateAppDomain(FnName);
	*pRetVal = (BSTR)pNewDomain->GetAppDomainName;
	this->m_NewlyCreatedAppDomains[std::wstring(*pRetVal)] = pNewDomain;
	return S_OK;
}