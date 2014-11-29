#include "stdafx.h"
#include "CppUnitTest.h"
#include "..\clr_host\ClrHost.h"
#pragma comment(lib, "clr_host.lib")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace NativeUnitTests
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			// TODO: Your test code here
			IUnmanagedHostPtr pClrHost;
			HRESULT hrBind = CClrHost::BindToRuntime(&pClrHost.GetInterfacePtr());
			Assert::AreSame<HRESULT>(S_OK, hrBind);
		}

	};
}