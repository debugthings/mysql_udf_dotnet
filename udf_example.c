/* Copyright (c) 2000, 2010, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "clr_host\ClrHost.h"
#pragma comment(lib, "clr_host.lib")
#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>



/* These must be right or mysqld will not find the symbol! */

//my_bool metaphon_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
//void metaphon_deinit(UDF_INIT *initid);
//char *metaphon(UDF_INIT *initid, UDF_ARGS *args, char *result,
//	       unsigned long *length, char *is_null, char *error);
//
//my_bool myfunc_double_init(UDF_INIT *, UDF_ARGS *args, char *message);
//double myfunc_double(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
//		     char *error);
//
//my_bool myfunc_int_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
//longlong myfunc_int(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
//		    char *error);
//
//my_bool sequence_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
// void sequence_deinit(UDF_INIT *initid);
//longlong sequence(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
//		   char *error);
//
//my_bool avgcost_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
//void avgcost_deinit( UDF_INIT* initid );
//void avgcost_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
//void avgcost_clear( UDF_INIT* initid, char* is_null, char *error );
//void avgcost_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
//double avgcost( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char *error );
//
//my_bool is_const_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
//char *is_const(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long
//               *length, char *is_null, char *error);

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

class CADMHostModule : public CAtlExeModuleT < CADMHostModule > { };
CADMHostModule _AtlModule;

extern "C"
{
	my_bool myfunc_int_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
	longlong myfunc_int(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
		char *error);
	void myfunc_int_deinit(UDF_INIT *initid);
	int RunApplication(IUnmanagedHostPtr &pClr, char *input);

	int RunApplication(IUnmanagedHostPtr &pClr, char *input)
	{
		// Get the default managed host
		IManagedHostPtr pManagedHost = pClr->DefaultManagedHost;

		// create a new AppDomain
		_bstr_t name(L"Second AppDomain");
		DWORD id = pManagedHost->CreateAppDomain(name);

		// get its host, and print to the screen
		IManagedHostPtr pSecondManagedHost = pClr->GetManagedHost(id);
		//pSecondManagedHost->Write(_bstr_t(L"Hello, World."));
		pSecondManagedHost->Run(_bstr_t(input));

		return 0;
	}


	IUnmanagedHostPtr pClr;

	my_bool myfunc_int_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
	{
		int returnCode = 0;
		HRESULT hrCoInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

		try
		{
			// bind the to CLR
			HRESULT hrBind = CClrHost::BindToRuntime(&pClr.GetInterfacePtr());
			if (FAILED(hrBind))
				_com_raise_error(hrBind);
			// start it up
			pClr->Start();
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

	longlong myfunc_int(UDF_INIT *initid, UDF_ARGS *args, char *is_null,
		char *error)
	{
		int returnCode = 0;
		try
		{
			// bind the to CLR
			
			// run the application
			returnCode = RunApplication(pClr, args->args[0]);
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

	void myfunc_int_deinit(UDF_INIT *initid)
	{
		int returnCode = 0;
		try
		{

			// finally, dispose of the managed hosts and shut down the runtime
			pClr->Stop();
		}
		catch (const _com_error &e)
		{
			const wchar_t *message = (wchar_t *)e.Description() == NULL ?
				L"" :
				(wchar_t *)e.Description();
			std::wcerr << L"Error 0x" << std::hex << e.Error() << L") : " << message << std::endl;

			returnCode = e.Error();
		}

		CoUninitialize();
	}
}