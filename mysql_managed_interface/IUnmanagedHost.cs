using System;
using System.Runtime.InteropServices;


namespace MySQLHostManager
{
    [ComVisible(true),
     Guid("3659e1f4-5003-4d1b-9c37-d82325428f94"),
     InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IUnmanagedHost
    {
        /// <summary>
        ///		Start the CLR
        /// </summary>
        void Start();

        /// <summary>
        ///		Stop the CLR
        /// </summary>
        void Stop();

        /// <summary>
        ///		Get the managed host of the default AppDomain
        /// </summary>
        IManagedHost DefaultManagedHost { get; }

        /// <summary>
        ///		Get the managed host for a specific AppDomain
        /// </summary>
        /// <param name="appDomain">AppDomain ID</param>
        IManagedHost GetManagedHost(int appDomain, string clrVersion);

        IManagedHost CreateAppDomainForQuery(string FnName);

        bool UnloadAppDomain(IManagedHost AppDomainName);
    }
}