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
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN 1
#endif


#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

using namespace ATL;
using namespace std;

_COM_SMARTPTR_TYPEDEF(ISupportErrorInfo, __uuidof(ISupportErrorInfo));
_COM_SMARTPTR_TYPEDEF(IErrorInfo, __uuidof(IErrorInfo));

/*
Global pointer to the unmanaged host. In our case it is the clr_host library.
This is a staticly linked class that is compiled into this UDF to avoid having tons of DLLs.
*/
IUnmanagedHostPtr pClrHost = NULL;

longlong RunInteger(IManagedHostPtr &pClr, _bstr_t &functionName, longlong input)
{
	return pClr->RunInteger(functionName, input);
}

double RunReal(IManagedHostPtr &pClr, _bstr_t &functionName, double input)
{
	return pClr->RunReal(functionName, input);
}

_bstr_t RunString(IUnmanagedHostPtr &pClr, _bstr_t &functionName, std::string &input)
{
	// Get the default managed host
	IManagedHostPtr pManagedHost = pClr->DefaultManagedHost;
	return pManagedHost->RunString(functionName, _bstr_t(input.c_str()));
}

// Create the proper error message to let MySQL know the UDF failed.
void errorMessage(const _com_error &e, char* message, BOOL isInit)
{

	auto hmod = LoadLibrary("C:\\Windows\\Microsoft.NET\\Framework\\v2.0.50727\\mscorrc.dll");
	std::stringstream errMessage;
	if (isInit)
		errMessage << "Unable to start CLR!";
	else
		errMessage << "Error in function execution!";

	errMessage << " Error (0x" << std::hex << e.Error() << ")";
	if (e.Description().length() > 0)
	{
		errMessage << " : " << e.Description();
	}
	else {

		auto ecode = e.Error() & ~(0x80130000);
		char strBuff[512];
		auto hr = LoadString(hmod, ecode + 0x6000, strBuff, 512);
		errMessage << " : " << strBuff;

	}
	errMessage << std::endl;

	ZeroMemory(message, MYSQL_ERRMSG_SIZE); // Need to clear out buffer to hold entire message

	if (errMessage.str().length() > MYSQL_ERRMSG_SIZE)
	{
		strcpy_s(message, 40, "Error message too long; run in debugger.");
	}
	else {
		auto s = errMessage.str();
		auto l = s.length();
		strcpy_s(message, l + 2, s.c_str()); // Add 2 because of the end line
	}
	FreeLibrary(hmod);
}

static CRITICAL_SECTION g_CritSec;

my_bool InitializeCLR(UDF_INIT *initid, UDF_ARGS *args, char *message)
{

	if (g_CritSec.DebugInfo == NULL)
	{
		InitializeCriticalSection(&g_CritSec);
	}
	if (args->arg_count < 2)
	{
		strcpy_s(message, 76, "You must supply at least two parameters. Parameter 1 must be the .NET class.");
		OutputDebugString(message);
		return 1;
	}

	int returnCode = 0;
	

	try
	{
		EnterCriticalSection(&g_CritSec);
		if (pClrHost == NULL)
		{
			HRESULT hrCoInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
			HRESULT hrBind = CClrHost::BindToRuntime(&pClrHost.GetInterfacePtr());
			if (FAILED(hrBind))
				_com_raise_error(hrBind);
			// start it up
			pClrHost->Start();
		}
		LeaveCriticalSection(&g_CritSec);
		if (args->arg_count > 0)
		{
			if (args->arg_type[0] == STRING_RESULT)
			{
				// Create new Appdomain and copy the name to the pointer to be used for the rest of the 
				// code execution.
				auto ret = pClrHost->CreateAppDomainForQuery(_bstr_t(args->args[0]));
				initid->ptr = (char*)&*ret; // Copy host pointer so it is not lost when out of scope.
			}
		}

		return 0;

	}
	catch (const _com_error &e)
	{
		errorMessage(e, message, TRUE);		
		OutputDebugString(message);
		return 1;
	}
	return 0;
}



extern "C"
{
	my_bool mysqldotnet_int_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
	{
		
		my_bool ret = InitializeCLR(initid, args, message);
		
		return ret;

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
			IManagedHostPtr mhp = (IManagedHost*)initid->ptr;

			// Start at the second parameter, as the first should be a string that tells us what class to execute.
			for (i = 1; i < args->arg_count; i++)
			{
				if (args->args[i] == NULL)
					continue;
				switch (args->arg_type[i]) {
				case INT_RESULT:			/* Add numbers */
					val += RunInteger(mhp, _bstr_t(args->args[0]), *((longlong*)args->args[i]));
					break;
				case REAL_RESULT:			/* Add numers as longlong */
					val += RunInteger(mhp, _bstr_t(args->args[0]), *((longlong*)args->args[i]));
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
			UNREFERENCED_PARAMETER(e);
			// Blanket error here. We are not worrying about what the exception is. We are trusting that this exception is bad.
			*error = 1;
			return 0;
		}

	}

	double mysqldotnet_real(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
		char *error)
	{
		int returnCode = 0;
		try
		{
			double val = 0;
			uint i;
			IManagedHostPtr mhp = (IManagedHost*)initid->ptr;

			// Start at the second parameter, as the first should be a string that tells us what class to execute.
			for (i = 1; i < args->arg_count; i++)
			{
				if (args->args[i] == NULL)
					continue;
				switch (args->arg_type[i]) {
				case STRING_RESULT:			/* Add string lengths */
					val += args->lengths[i];
					break;
				case INT_RESULT:			/* Add numbers */
					val += (double)RunInteger(mhp, _bstr_t(args->args[0]), *((longlong*)args->args[i]));
					break;
				case REAL_RESULT:			/* Add numers as longlong */
					val += RunReal(mhp, _bstr_t(args->args[0]), *((double*)args->args[i]));
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
			UNREFERENCED_PARAMETER(e);
			// Blanket error here. We are not worrying about what the exception is. We are trusting that this exception is bad.
			*error = 1;
			return 0;
		}
		return 0;
	}

	char* mysqldotnet_string(UDF_INIT *initid __attribute__((unused)),
		UDF_ARGS *args, char *result, unsigned long *length,
		char *is_null, char *error __attribute__((unused)))
	{
		try
		{
			uint i;
			std::string stringResult;

			// Start at the second parameter, as the first should be a string that tells us what class to execute.
			for (i = 1; i < args->arg_count; i++)
			{
				stringResult.clear();
				if (args->args[i] == NULL)
					continue;
				switch (args->arg_type[i]) {
				case STRING_RESULT:			/* Add string lengths */
					stringResult.assign((LPCSTR)RunString(pClrHost, _bstr_t(args->args[0]), std::string(args->args[i])));
					strcpy_s(result, stringResult.length(), stringResult.c_str());
					break;
				case INT_RESULT:
					stringResult.assign((LPCSTR)RunString(pClrHost, _bstr_t(args->args[0]), std::to_string(*(longlong*)args->args[i])));
					strcpy_s(result, stringResult.length(), stringResult.c_str());
					break;
				case REAL_RESULT:			/* Add numers as longlong */
					stringResult.assign((LPCSTR)RunString(pClrHost, _bstr_t(args->args[0]), std::to_string(*(double*)args->args[i])));
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
			UNREFERENCED_PARAMETER(e);
			// Blanket error here. We are not worrying about what the exception is. We are trusting that this exception is bad.
			*error = 1;
			return 0;

		}
		return 0;
	}


	void mysqldotnet_int_deinit(UDF_INIT *initid)
	{
		pClrHost->UnloadAppDomain((IManagedHost*)initid->ptr);
		((IManagedHost*)initid->ptr)->Release();
	}

	void mysqldotnet_real_deinit(UDF_INIT *initid)
	{
		pClrHost->UnloadAppDomain((IManagedHost*)initid->ptr);
		((IManagedHost*)initid->ptr)->Release();
	}

	void mysqldotnet_string_deinit(UDF_INIT *initid)
	{
		pClrHost->UnloadAppDomain((IManagedHost*)initid->ptr);
		((IManagedHost*)initid->ptr)->Release();
	}
}