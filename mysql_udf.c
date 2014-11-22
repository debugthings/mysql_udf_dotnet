

#include "clr_host\ClrHost.h"
#pragma comment(lib, "clr_host.lib")
#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>

#ifndef STRICT
#  define STRICT
#endif

#ifndef WINVER
#  define WINVER 0x0400
#endif

#ifndef _WIN32_WINNT
#  define _WIN32_WINNT 0x0500
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _WINDLL 0

using namespace ATL;
using namespace std;

_COM_SMARTPTR_TYPEDEF(ISupportErrorInfo, __uuidof(ISupportErrorInfo));
_COM_SMARTPTR_TYPEDEF(IErrorInfo, __uuidof(IErrorInfo));


IUnmanagedHostPtr pClrHost = NULL;

_bstr_t RunString(IUnmanagedHostPtr &pClr, std::string &input)
{
	// Get the default managed host
	IManagedHostPtr pManagedHost = pClr->DefaultManagedHost;
	return pManagedHost->RunString(L"myfunc", _bstr_t(input.c_str()));
}
my_bool InitializeCLR(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	int returnCode = 0;
	HRESULT hrCoInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	try
	{
		if (pClrHost == NULL)
		{
			HRESULT hrBind = CClrHost::BindToRuntime(&pClrHost.GetInterfacePtr());
			if (FAILED(hrBind))
				_com_raise_error(hrBind);
			// start it up
			pClrHost->Start();
		}
		if (args->arg_count > 0)
		{
			if (args->arg_type[0] == STRING_RESULT)
			{
				auto ret = pClrHost->CreateAppDomainForQuery(_bstr_t(args->args[0]));
				initid->ptr = (char*)ret.copy();
			}
		}

	}
	catch (const _com_error &e)
	{
		const wchar_t *message = (wchar_t *)e.Description() == NULL ?
			L"" :
			(wchar_t *)e.Description();
		std::wcerr << L"Error 0x" << std::hex << e.Error() << L") : " << message << std::endl;

		returnCode = e.Error();
	}

	//initid->ptr = reinterpret_cast<char *>(&pClrHost);
	return 0;
}
longlong RunInteger(IManagedHostPtr &pClr, std::string &functionName, longlong input)
{
	return pClr->RunInteger(L"MySQLCustomClass.CustomMySQLClass", input);
}

double RunReal(IManagedHostPtr &pClr, double input)
{
	return pClr->RunReal(L"MySQLCustomClass.CustomMySQLClass", input);
}

//BOOL WINAPI DllMain(
//	_In_  HINSTANCE hinstDLL,
//	_In_  DWORD fdwReason,
//	_In_  LPVOID lpvReserved
//	)
//{
//
//	int returnCode = 0;
//	HRESULT hrCoInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
//	switch (fdwReason)
//	{
//	case DLL_PROCESS_ATTACH:
//		try
//		{
//			if (pClrHost == NULL)
//			{
//				HRESULT hrBind = CClrHost::BindToRuntime(&pClrHost.GetInterfacePtr());
//				if (FAILED(hrBind))
//					_com_raise_error(hrBind);
//				// start it up
//				pClrHost->Start();
//			}
//		}
//		catch (const _com_error &e)
//		{
//			const wchar_t *message = (wchar_t *)e.Description() == NULL ?
//				L"" :
//				(wchar_t *)e.Description();
//			std::wcerr << L"Error 0x" << std::hex << e.Error() << L") : " << message << std::endl;
//			returnCode = e.Error();
//		}
//
//	default:
//		break;
//
//	}
//	return true;
//}

extern "C"
{
	my_bool mysqldotnet_int_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
	{
		return InitializeCLR(initid, args, message);
	}

	my_bool mysqldotnet_real_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
	{
		return InitializeCLR(initid, args, message);
	}

	my_bool mysqldotnet_string_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
	{
		return InitializeCLR(initid, args, message);
	}


	long long mysqldotnet_int(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
		char *error)
	{
		int returnCode = 0;
		try
		{
			longlong val = 0;
			uint i;
			IManagedHostPtr mhp = pClrHost->GetSpecificManagedHost(BSTR((BSTR*)initid->ptr));


			for (i = 1; i < args->arg_count; i++)
			{
				if (args->args[i] == NULL)
					continue;
				switch (args->arg_type[i]) {
				case INT_RESULT:			/* Add numbers */
					val += RunInteger(mhp, std::string(args->args[0]), *((longlong*)args->args[i]));
					break;
				case REAL_RESULT:			/* Add numers as longlong */
					val += RunInteger(mhp, std::string(args->args[0]), *((longlong*)args->args[i]));
					break;
				default:
					break;
				}
			}
			return val;
			// run the application
		}
		catch (const _com_error &e)
		{
			const wchar_t *message = (wchar_t *)e.Description() == NULL ?
				L"" :
				(wchar_t *)e.Description();
			std::wcerr << L"Error 0x" << std::hex << e.Error() << L") : " << message << std::endl;

			returnCode = e.Error();
		}
		return 0;
	}

	double mysqldotnet_real(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
		char *error)
	{
		int returnCode = 0;
		try
		{
			double val = 0;
			uint i;
			IManagedHostPtr mhp = pClrHost->GetSpecificManagedHost(BSTR(initid->ptr));

			for (i = 1; i < args->arg_count; i++)
			{
				if (args->args[i] == NULL)
					continue;
				switch (args->arg_type[i]) {
				case STRING_RESULT:			/* Add string lengths */
					val += args->lengths[i];
					break;
				case INT_RESULT:			/* Add numbers */
					val += (double)RunInteger(mhp, std::string(args->args[0]), *((longlong*)args->args[i]));
					break;
				case REAL_RESULT:			/* Add numers as longlong */
					val += RunReal(mhp, *((double*)args->args[i]));
					break;
				default:
					break;
				}
			}
			return val;
			// run the application
		}
		catch (const _com_error &e)
		{
			const wchar_t *message = (wchar_t *)e.Description() == NULL ?
				L"" :
				(wchar_t *)e.Description();
			std::wcerr << L"Error 0x" << std::hex << e.Error() << L") : " << message << std::endl;

			returnCode = e.Error();
		}
		return 0;
	}

	char* mysqldotnet_string(UDF_INIT *initid __attribute__((unused)),
		UDF_ARGS *args, char *result, unsigned long *length,
		char *is_null, char *error __attribute__((unused)))
	{
		try
		{
			longlong val = 0;
			uint i;
			std::string stringResult;
			for (i = 0; i < args->arg_count; i++)
			{
				stringResult.clear();
				if (args->args[i] == NULL)
					continue;
				switch (args->arg_type[i]) {
				case STRING_RESULT:			/* Add string lengths */

					stringResult.assign((LPCSTR)RunString(pClrHost, std::string(args->args[i])));
					strcpy_s(result, stringResult.length(), stringResult.c_str());
					break;
				case INT_RESULT:
					stringResult.assign((LPCSTR)RunString(pClrHost, std::to_string(*(longlong*)args->args[i])));
					strcpy_s(result, stringResult.length(), stringResult.c_str());
					break;
				case REAL_RESULT:			/* Add numers as longlong */
					stringResult.assign((LPCSTR)RunString(pClrHost, std::to_string(*(double*)args->args[i])));
					strcpy_s(result, stringResult.length(), stringResult.c_str());
					break;
				default:
					break;
				}
			}
			return result;
			// run the application
		}
		catch (const _com_error &e)
		{
			const wchar_t *message = (wchar_t *)e.Description() == NULL ?
				L"" :
				(wchar_t *)e.Description();
			std::wcerr << L"Error 0x" << std::hex << e.Error() << L") : " << message << std::endl;

		}
		return 0;
	}

	void mysqldotnet_int_deinit(UDF_INIT *initid)
	{
		pClrHost->DefaultManagedHost->Unload(BSTR(initid->ptr));
	}

	void mysqldotnet_real_deinit(UDF_INIT *initid)
	{
		//pClrHost->Release();
	}

	void mysqldotnet_string_deinit(UDF_INIT *initid)
	{
		//pClrHost->Release();
	}
}