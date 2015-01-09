using System;
using System.Runtime.InteropServices;

namespace MySQLHostManager
{
    [ComVisible(true),
     Guid("0961af1d-ebb7-4f7d-ab4a-d4234b74caca"),
     InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IManagedHost
    {
        /// <summary>
        ///		Create a new AppDomain
        /// </summary>
        /// <param name="name">name of the AppDomain to create</param>
        /// <returns>ID of the new AppDomain</returns>
        IManagedHost CreateAppDomain(string typeName);

        /// <summary>
        ///		Clean up the AppDomianManager
        /// </summary>
        void Dispose();

        /// <summary>
        ///		Set the unmanaged host for the AppDomainManager to work with
        /// </summary>
        /// <param name="unmanagedHost">unmanaged half of the host</param>
        void SetUnmanagedHost(IUnmanagedHost unmanagedHost);

        Int64 RunInteger(string functionName, Int64 value);
        Int64 RunIntegers(string functionName, Int64[] values);


        double RunReal(string functionName, double value);
        double RunReals(string functionName, double[] values);

        [return: MarshalAs(UnmanagedType.BStr)]
        string RunString(string functionName, string value);
        [return: MarshalAs(UnmanagedType.BStr)]
        string RunStrings(string functionName, string[] values);

        [return: MarshalAs(UnmanagedType.BStr)]
        string GetCLR();

        string GetAssemblyCLRVersion(string AssemblyName);

        string GetAppDomainName { get; }

        bool Unload(string FriendlyName);

        string MultiKeyword { get;  }
        int DefaultCodepage { get; }
    }

}