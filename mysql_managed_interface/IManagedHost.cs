using System;
using System.Runtime.InteropServices;

namespace mysql_managed_interface
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
        int CreateAppDomain([MarshalAs(UnmanagedType.BStr)]string name);

        /// <summary>
        ///		Clean up the AppDomianManager
        /// </summary>
        void Dispose();

        /// <summary>
        ///		Set the unmanaged host for the AppDomainManager to work with
        /// </summary>
        /// <param name="unmanagedHost">unmanaged half of the host</param>
        void SetUnmanagedHost(IUnmanagedHost unmanagedHost);

        /// <summary>
        ///		Write a message
        /// </summary>
        /// <param name="message">message to write</param>
        void Write([MarshalAs(UnmanagedType.BStr)]string message);


        /// <summary>
        ///		Write a message
        /// </summary>
        /// <param name="message">message to write</param>
        /// 
        [return: MarshalAs(UnmanagedType.I8)]
        Int64 Run([MarshalAs(UnmanagedType.I8)]Int64 path);
    }

}