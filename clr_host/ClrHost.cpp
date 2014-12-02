/*
 *
 * ClrHost.cpp this is the base file for the .NET Hosting API native interface.
 *
 */

#include "ClrHost.h"

#define MAXSTRING 128
#define V4 "v4" // Incase we are using an older (depreciated) version of the RegistryKey
#define V40 "v4.0"
#define V20 "v2.0"
#define V40L L"v4.0"
#define V20L L"v2.0"
/*
 * These are the hard coded values for the AppDomain managers.
 */
const wchar_t *CClrHost::AppDomainManagerAssembly20 = L"MySQLHostManager, Version=2.0.0.0, Culture=neutral, PublicKeyToken=71c4a5d4270bd29c, processorArchitecture=MSIL";
const wchar_t *CClrHost::AppDomainManagerAssembly40 = L"MySQLHostManager, Version=4.0.0.0, Culture=neutral, PublicKeyToken=71c4a5d4270bd29c, processorArchitecture=MSIL";
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
	// free the AppDomainManagers
	for (AppDomainManagerMap::iterator iAdm = m_appDomainManagers.begin(); iAdm != m_appDomainManagers.end(); iAdm++)
		iAdm->second->Release();

	// release the CLR
	if (m_pClrControl != NULL)
		m_pClrControl->Release();
	return;
}

/// <summary>
///     Finish setting up the CLR host
/// </summary>
#pragma warning( disable : 4996 )
HRESULT CClrHost::FinalConstruct()
{

	ICLRMetaHost       *pMetaHost = NULL;
	ICLRMetaHostPolicy *pMetaHostPolicy = NULL;
	HRESULT hr;
	ICLRRuntimeInfo *info = NULL;
	ICLRRuntimeHost *m_pClr = NULL;

	// Use this to find out what versions of the CLR are installed.
	// We will prefer to use v4.0 for now; this is not future proof as v5 may come out soon
	HKEY netFrame;
	hr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\NET Framework Setup\\NDP", NULL, KEY_READ | KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &netFrame);
	int keyIndex = 0;
	char keys[10][MAXSTRING]; // Store 10 keys x 128bytes
	BOOL has40 = FALSE;
	WCHAR version[MAXSTRING];
	DWORD versionSize = 0;

	while (hr == 0)
	{
		DWORD keyLen = MAXSTRING;
		ZeroMemory(keys[keyIndex], MAXSTRING);
		hr = RegEnumKeyEx(netFrame, keyIndex, keys[keyIndex], &keyLen, NULL, NULL, NULL, NULL);
		if (hr == ERROR_ACCESS_DENIED)
			return hr;
		if (!has40)
		{
			has40 = strcmp(V4, keys[keyIndex]);
		}
		if (keyIndex++ > 10)
			break; // There shouldn't be 10 keys here, but some high random number is better than letting it go forever.
	}

	bool disableSxS = FALSE;
	// If we have 4.0 try to use the 4.0 binding policy.
	if (has40)
	{
		DWORD actFlags = 0;
		hr = CLRCreateInstance(CLSID_CLRMetaHostPolicy, IID_ICLRMetaHostPolicy,
			(LPVOID*)&pMetaHostPolicy);
		if (FAILED(hr))
			return hr;

		hr = pMetaHostPolicy->GetRequestedRuntime(
			METAHOST_POLICY_USE_PROCESS_IMAGE_PATH,
			NULL,
			NULL,
			NULL,
			&versionSize,
			NULL,
			NULL,
			&actFlags,
			IID_ICLRRuntimeInfo,
			reinterpret_cast<LPVOID *>(&info));
		if (FAILED(hr))
			return hr;

		// Check the preferred version
		hr = info->GetVersionString(version, &versionSize);
		if (FAILED(hr))
			return hr;

		disableSxS = (actFlags & METAHOST_CONFIG_FLAGS_LEGACY_V2_ACTIVATION_POLICY_MASK) & METAHOST_CONFIG_FLAGS_LEGACY_V2_ACTIVATION_POLICY_TRUE; // Disable SxS

		hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost,
			(LPVOID*)&pMetaHost);
		if (FAILED(hr))
			return hr;
	}

	// If we don't have 4.0, bind the old way to the latest version (v2.0.50727)
	if (!has40) {

		// In the case of our binding we will force the required version.
		// If we fail this kills the initilization of the CLR
		hr = GetCORRequiredVersion(version, MAXSTRING, &versionSize);
		if (FAILED(hr))
			return hr;

		hr = CorBindToRuntimeEx(version,
			NULL,
			0,
			CLSID_CLRRuntimeHost,
			IID_ICLRRuntimeHost,
			reinterpret_cast<LPVOID *>(&m_pClr));

		if (FAILED(hr))
			return hr;

		// Pulled out common startup items.
		return SetupCLR(m_pClr, version, version);

	}

	pMetaHostPolicy->Release();
	// Enumeration example from COM books.

	bool runtimesLoaded = false;
	//pMetaHost->EnumerateLoadedRuntimes(GetCurrentProcess(), &pRtEnum);

	WCHAR strName[MAXSTRING];
	DWORD len = MAXSTRING;

	ULONG fetched = 0;
	IEnumUnknown * pRtEnum = NULL;


	pMetaHost->EnumerateLoadedRuntimes(GetCurrentProcess(), &pRtEnum);
	while ((hr = pRtEnum->Next(1, (IUnknown **)&info, &fetched)) == S_OK && fetched > 0)
	{
		// If the runtime is loaded in MySQL already, then we may not have control over it.
		// The right thing to do is check to see if the CLR that is loaded matches the 
		// IManagedHost IID and we can then call GetUnManagedHost to set our pointer.
		runtimesLoaded = true;
	}
	pRtEnum->Release();
	pRtEnum = NULL;

	// If no runtimes are loaded we will make sure to load them all based on the policy.
	// This will set the default runtime as the last CLR to be loaded.
	// At the time of writing this application it is 4.5 (4.0)
	if (!runtimesLoaded)
	{
		pMetaHost->EnumerateInstalledRuntimes(&pRtEnum);
		while ((hr = pRtEnum->Next(1, (IUnknown **)&info, &fetched)) == S_OK && fetched > 0)
		{
			ZeroMemory(strName, sizeof(strName));
			info->GetVersionString(strName, &len);

			// If we are disabling side by side execution (useLegacyV2RuntimeActivationPolicy) then only load the speficied CLR
			// If we haven't specified SxS policy then load all.
			if (((StrCmpW(strName, version) == 0) & disableSxS) || !disableSxS)
			{
				hr = info->GetInterface(CLSID_CLRRuntimeHost,
					IID_ICLRRuntimeHost,
					reinterpret_cast<LPVOID *>(&m_pClr));
				if (FAILED(hr))
					return hr;

				// Pulled out common startup items.
				hr = SetupCLR(m_pClr, strName, version);
				if (FAILED(hr))
					return hr;
			}

		}
		pRtEnum->Release();
	}
	pMetaHost->Release();
	return S_OK;
}

HRESULT CClrHost::SetupCLR(ICLRRuntimeHost *m_pClr, PCWSTR strCmp, PCWSTR version)
{
	auto strCm = std::wstring(strCmp);
	auto prefVer = std::wstring(version);

	m_CLRRuntimeMap[strCm.substr(0, 4)] = m_pClr;

	HRESULT hrClrControl = m_pClr->GetCLRControl(&m_pClrControl);
	if (FAILED(hrClrControl))
		return hrClrControl;


	// setup the AppDomainManager for the proper version of the CLR
	// we are supporting both v2.0 and v4.0 so we will provide unique AppDomainManagers for both.
	// This of course is 
	if (strCm.substr(0, 4) == V20L)
	{
		HRESULT hrSetAdm = m_pClrControl->SetAppDomainManagerType(AppDomainManagerAssembly20, AppDomainManagerType);
		if (FAILED(hrSetAdm))
			return hrSetAdm;
	}
	else if (strCm.substr(0, 4) == V40L)
	{
		HRESULT hrSetAdm = m_pClrControl->SetAppDomainManagerType(AppDomainManagerAssembly40, AppDomainManagerType);
		if (FAILED(hrSetAdm))
			return hrSetAdm;
	}

	// set ourselves up as the host control
	HRESULT hrHostControl = m_pClr->SetHostControl(static_cast<IHostControl *>(this));
	if (FAILED(hrHostControl))

		return hrHostControl;
	// get the host protection manager
	ICLRHostProtectionManager *pHostProtectionManager = NULL;
	HRESULT hrGetProtectionManager = m_pClrControl->GetCLRManager(
		IID_ICLRHostProtectionManager,
		reinterpret_cast<void **>(&pHostProtectionManager));
	if (FAILED(hrGetProtectionManager))
		return hrGetProtectionManager;

	// setup host proctection to disallow any threading from partially trusted code.
	// Why? well, if a thread is allowed to hang indefinitely the command could get stuck.
	HRESULT hrHostProtection = pHostProtectionManager->SetProtectedCategories(
		(EApiCategories)(eSynchronization | eSelfAffectingThreading | eSelfAffectingProcessMgmt
		| eExternalProcessMgmt | eExternalThreading | eUI));
	pHostProtectionManager->Release();

	if (FAILED(hrHostProtection))
		return hrHostProtection;

	// Set the default AppDomain manager to the preferred version
	// WARNING this has the un intended side effect of blowing up the CLR if v2.0 is first
	// Why? Well in order not to implement my own binding policies I am going to create the AppDomain on the default host
	// unless the clrversion attribute is set on the assembly.
	if (strCm.substr(0, 4) == prefVer.substr(0, 4))
	{
		this->m_lastCLR.assign(strCm.substr(0, 4));
	}
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
	//
	_ASSERTE(!m_started);
	if (!m_started)
	{

		_ASSERTE(&m_CLRRuntimeMap != NULL);
		for (auto &x : m_CLRRuntimeMap)
		{

			ICLRRuntimeHost *m_pClr = x.second;

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
	auto clrVersion = pAppMgr->GetAssemblyCLRVersion(FnName);
	auto domManager = this->m_appDomainManagers[std::wstring(clrVersion)];
	IManagedHostPtr pNewDomain = domManager->CreateAppDomain(FnName);
	*pRetVal = (BSTR)pNewDomain->GetAppDomainName;
	this->m_NewlyCreatedAppDomains[std::wstring(*pRetVal)] = pNewDomain;
	return S_OK;
}