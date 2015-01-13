
#include "clr_host\ClrHost.h"
#pragma comment(lib, "clr_host.lib")


#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>
#include <map>
#include <vector>

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

auto stringMap = new std::map<std::wstring, std::vector<char*>>();

char* g_multikeyword = nullptr;
int g_codepage = 0;

longlong RunInteger(IManagedHostPtr &pClr, _bstr_t &functionName, longlong input)
{
	return pClr->RunInteger(functionName, input);
}

double RunReal(IManagedHostPtr &pClr, _bstr_t &functionName, double input)
{
	return pClr->RunReal(functionName, input);
}

_bstr_t RunString(IManagedHostPtr &pClr, _bstr_t &functionName, char *input, int size, int *codepage)
{
	if (*codepage == 0)
	{
		*codepage = g_codepage;
	}
	int sizeplusterm = size + 1;
	wchar_t* buffer = new wchar_t[sizeplusterm] {};
	int unicodecheck = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK;
	IsTextUnicode(input, size, &unicodecheck);
	if ((unicodecheck > 0) | (*codepage == CP_WINUNICODE))
	{
		for (int i = 0, j = 0; i < size / 2; i++, j += 2)
		{
			buffer[i] |= (((wchar_t)input[j]) << 0x8) | ((wchar_t)(input[j + 1]));
		}
	}
	else {
		MultiByteToWideChar(*codepage, NULL, input, size, buffer, sizeplusterm);
	}
	return pClr->RunString(functionName, buffer);
}

longlong RunIntegers(IManagedHostPtr &pClr, _bstr_t &functionName, longlong* input, int args)
{
	SAFEARRAYBOUND rgsabound[1];
	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = args;
	SAFEARRAY* sa = SafeArrayCreate(VT_I8, 1, rgsabound);
	SafeArrayLock(sa);
	sa->pvData = input;
	SafeArrayUnlock(sa);
	return pClr->RunIntegers(functionName, sa);
}

double RunReals(IManagedHostPtr &pClr, _bstr_t &functionName, double* input, int args)
{
	SAFEARRAYBOUND rgsabound[1];
	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = args;
	SAFEARRAY* sa = SafeArrayCreate(VT_R8, 1, rgsabound);
	SafeArrayLock(sa);
	sa->pvData = input;
	SafeArrayUnlock(sa);
	return pClr->RunReals(functionName, sa);
}

_bstr_t RunStrings(IManagedHostPtr &pClr, _bstr_t &functionName, char** input, unsigned long *lengths, uint args, int *codepage)
{
	int codepageIndex = 2;
	if (*codepage != 0)
	{
		++codepageIndex; // Increase index if we have an explicit code page.
	}
	else
	{
		*codepage = g_codepage; // If there is no code page we fall back to the default.
	}


	SAFEARRAYBOUND rgsabound[1];
	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = args - (codepageIndex - 1);
	SAFEARRAY* sa = SafeArrayCreate(VT_BSTR, 1, rgsabound);
	HRESULT hr = SafeArrayLock(sa);
	for (uint ix = codepageIndex; ix <= args; ix++)
	{
		int txtLen = lengths[ix] + 1;
		auto txt = new wchar_t[txtLen] {};
		int unicodecheck = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK;
		IsTextUnicode(input[ix], lengths[ix], &unicodecheck);
		if ((unicodecheck > 0) | (*codepage == CP_WINUNICODE))
		{
			for (size_t i = 0, j = 0; i < lengths[ix] / 2; i++, j += 2)
			{
				txt[i] |= ((reinterpret_cast<wchar_t>(input[j])) << 0x8) | (reinterpret_cast<wchar_t>((input[j + 1])));
			}
		}
		else {
			MultiByteToWideChar(*codepage, NULL, input[ix], lengths[ix], txt, txtLen);
		}
		//((BSTR*)sa->pvData)[ix - codepageIndex] = SysAllocString(txt);
		((BSTR*)sa->pvData)[ix - codepageIndex] = *(new _bstr_t(txt));
		delete txt;
	}
	hr = SafeArrayUnlock(sa);
	auto retstring = pClr->RunStrings(functionName, sa);
	SafeArrayDestroy(sa);
	return retstring;
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
			if (g_codepage != 0)
			{
				g_codepage = pClrHost->GetDefaultManagedHost()->GetDefaultCodepage();
			}
			if (g_multikeyword == nullptr)
			{
				_bstr_t multikeyword = _bstr_t(pClrHost->GetDefaultManagedHost()->GetMultiKeyword());
				g_multikeyword = new char[multikeyword.length() + 1] {};
				WideCharToMultiByte(CP_UTF8, NULL, multikeyword, -1, g_multikeyword, multikeyword.length(), NULL, NULL);
			}
		}
		LeaveCriticalSection(&g_CritSec);
		if (args->arg_count > 0)
		{
			if (args->arg_type[0] == STRING_RESULT)
			{
				// Create new Appdomain and copy the name to the pointer to be used for the rest of the 
				// code execution.
				auto ret = pClrHost->CreateAppDomainForQuery(_bstr_t(args->args[0]));
				auto name = std::wstring(ret->GetAppDomainName);
				initid->ptr = reinterpret_cast<char*>(&*ret); // Copy host pointer so it is not lost when out of scope.
				stringMap->insert(std::pair<std::wstring, std::vector<char*>>(name, std::vector<char*>()));
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

template<typename _type>
_type getIntegerForArray(char* arg, Item_result type)
{
	switch (type) {
	case INT_RESULT:			/* Add numbers */
		return (_type)*((longlong*)arg);
		break;
	case REAL_RESULT:			/* Add numers as longlong */
		return (_type)*((double*)arg);
		break;
	case STRING_RESULT:
		return (_type)strlen(arg);
		break;
	default:
		return 0;
		break;
	}
}



extern "C"
{
	my_bool mysqldotnet_int_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
	{
		for (size_t i = 0; i < args->arg_count; i++)
		{
			if (args->arg_type[i] == DECIMAL_RESULT)
			{
				args->arg_type[i] = REAL_RESULT;
			}
		}
		my_bool ret = InitializeCLR(initid, args, message);

		return ret;

	}

	my_bool mysqldotnet_real_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
	{
		for (size_t i = 0; i < args->arg_count; i++)
		{
			if (args->arg_type[i] == DECIMAL_RESULT)
			{
				args->arg_type[i] = REAL_RESULT;
			}
		}
		return InitializeCLR(initid, args, message);
	}

	my_bool mysqldotnet_string_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
	{
		for (size_t i = 0; i < args->arg_count; i++)
		{
			if (args->arg_type[i] == DECIMAL_RESULT)
			{
				args->arg_type[i] = REAL_RESULT;
			}
		}
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
			IManagedHostPtr mhp = reinterpret_cast<IManagedHost*>(initid->ptr);;
			char* argName = args->args[0];
			++args->args;
			++args->arg_type;
			// Start at the second parameter, as the first should be a string that tells us what class to execute.
			for (i = 0; i < args->arg_count - 1; i++)
			{
				BOOL stopLoop = FALSE;
				if (args->args[i] == NULL)
					continue;
				switch (args->arg_type[i]) {
				case INT_RESULT:			/* Add numbers */
					val += RunInteger(mhp, _bstr_t(argName), getIntegerForArray<longlong>(args->args[i], args->arg_type[i]));
					break;
				case REAL_RESULT:			/* Add numers as longlong */
					val += RunInteger(mhp, _bstr_t(argName), getIntegerForArray<longlong>(args->args[i], args->arg_type[i]));
					break;
				case STRING_RESULT:
					if (strcmp((char*)args->args[i], g_multikeyword) == 0)
					{
						++args->args;
						++args->arg_type;
						longlong* longArray = new longlong[args->arg_count - 2];
						for (size_t j = 0; j < args->arg_count - 2; j++)
						{
							longArray[j] = getIntegerForArray<longlong>(args->args[j], args->arg_type[j]);
						}
						val += RunIntegers(mhp, _bstr_t(argName), longArray, args->arg_count - 2);
						delete[] longArray;
						stopLoop = TRUE;
					}

					break;
				default:
					break;
				}
				if (stopLoop == TRUE)
				{
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
			IManagedHostPtr mhp = reinterpret_cast<IManagedHost*>(initid->ptr);

			char* argName = args->args[0];
			++args->args;
			++args->arg_type;

			// Start at the second parameter, as the first should be a string that tells us what class to execute.
			for (i = 0; i < args->arg_count - 1; i++)
			{
				BOOL stopLoop = FALSE;
				if (args->args[i] == NULL)
					continue;
				switch (args->arg_type[i]) {
				case INT_RESULT:			/* Add numbers */
					val += RunReal(mhp, _bstr_t(argName), getIntegerForArray<double>(args->args[i], args->arg_type[i]));
					break;
				case REAL_RESULT:			/* Add numers as longlong */
					val += RunReal(mhp, _bstr_t(argName), getIntegerForArray<double>(args->args[i], args->arg_type[i]));
					break;
				case STRING_RESULT:
					if (strcmp((char*)args->args[i], g_multikeyword) == 0)
					{
						++args->args;
						++args->arg_type;
						double* longArray = new double[args->arg_count - 2];
						for (size_t j = 0; j < args->arg_count - 2; j++)
						{
							longArray[j] = getIntegerForArray<double>(args->args[j], args->arg_type[j]);
						}
						val = RunReals(mhp, _bstr_t(argName), longArray, args->arg_count - 2);
						delete[] longArray;
						return val;
					}
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

	char  *mysqldotnet_string(UDF_INIT *initid __attribute__((unused)),
		UDF_ARGS *args, char *result, unsigned long *length,
		char *is_null, char *error __attribute__((unused)))
	{
		try
		{
			ZeroMemory(result, *length);
			uint i;
			long lastlen = 0;
			std::string stringResult;
			IManagedHostPtr mhp = reinterpret_cast<IManagedHost*>(initid->ptr);
			_bstr_t argName = _bstr_t(args->args[0]);
			_bstr_t ptrCpy = _bstr_t();
			long len = 0;
			std::string s;
			int codepage = 0;
			// Start at the second parameter, as the first should be a string that tells us what class to execute.
			for (i = 1; i < args->arg_count; i++)
			{
				BOOL stopLoop = FALSE;
				stringResult.clear();
				BOOL useUnicode = FALSE;
				if (args->args[i] == NULL)
					continue;
				if (args->arg_type[i] == STRING_RESULT) {
					if ((strcmp(static_cast<char*>(args->args[i]), g_multikeyword) == 0) && ((i == 1) || ((codepage > 0) && (i == 2))))
					{
						// If multi is in the first position then we are MULTI valued
						// If multi is in the second position and the code page is set then we are multivalued
						ptrCpy = RunStrings(mhp, argName, args->args, args->lengths, args->arg_count - 1, &codepage);
						stopLoop = TRUE;
					}
					else {
						ptrCpy = RunString(mhp, argName, args->args[i], args->lengths[i], &codepage);
					}
					len = ptrCpy.length(); // Hard limit at 1MB string
					lastlen = len;

					char* buffer = new char[len + 1] {};
					lastlen = len + 1;
					WideCharToMultiByte(CP_UTF8, NULL, ptrCpy, -1, buffer, lastlen, NULL, NULL);
					ulong bufferSize = strlen(buffer) * sizeof(char);
					lastlen = bufferSize;

					if (bufferSize <= *length)
					{
						memcpy_s(result, *length, buffer, lastlen);
						delete buffer;
					}
					else
					{
						auto map = stringMap->at(std::wstring(mhp->GetAppDomainName));
						map.push_back(buffer);
						result = buffer;
					}
					if (stopLoop == TRUE)
					{
						*length = lastlen;
						return result;
					}
				}
				else if (INT_RESULT) {

					if (i == 1)
					{
						codepage = *(int*)args->args[i];
						continue; // If it's the first argument then we need to set the code page
					}
					s = std::to_string(*(longlong*)args->args[i]);
					stringResult.assign((LPCSTR)RunString(mhp, argName, (char*)s.c_str(), s.length(), &codepage));

					strcpy_s(result, *length, stringResult.c_str());
					if (*length >= stringResult.length())
					{
						lastlen = stringResult.length();
					}
				}
				else if (REAL_RESULT) {
					/* Add numers as longlong */
					s = std::to_string(*(double*)args->args[i]);
					stringResult.assign((LPCSTR)RunString(mhp, argName, (char*)s.c_str(), s.length(), &codepage));
					strcpy_s(result, *length, stringResult.c_str());
					if (*length >= stringResult.length())
					{
						lastlen = stringResult.length();
					}
				}
			}
			*length = lastlen;
			return result;
			// run the application
		}
		catch (const _com_error &e)
		{
			UNREFERENCED_PARAMETER(e);
			// Blanket error here. We are not worrying about what the exception is. We are trusting that this exception is bad.
			char msg[1024];
			errorMessage(e, msg, FALSE);
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
		auto mhp = (IManagedHost*)initid->ptr;
		auto map = stringMap->at(std::wstring(mhp->GetAppDomainName));
		for (auto &it : map)
		{
			delete it;
		}
		pClrHost->UnloadAppDomain((IManagedHost*)initid->ptr);
		((IManagedHost*)initid->ptr)->Release();
	}
}