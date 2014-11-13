using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Security;
using System.Security.Permissions;
using System.Security.Policy;

namespace mysql_managed_interface
{
    class MySQLHostManager: AppDomainManager, IManagedHost
    {

        public static StrongName CreateStrongName(Assembly assembly)
        {
            if (assembly == null)
                throw new ArgumentNullException("assembly");

            AssemblyName assemblyName = assembly.GetName();
            Debug.Assert(assemblyName != null, "Could not get assembly name");

            // get the public key blob
            byte[] publicKey = assemblyName.GetPublicKey();
            if (publicKey == null || publicKey.Length == 0)
                throw new InvalidOperationException("Assembly is not strongly named");

            StrongNamePublicKeyBlob keyBlob = new StrongNamePublicKeyBlob(publicKey);

            // and create the StrongName
            return new StrongName(keyBlob, assemblyName.Name, assemblyName.Version);
        }

        private  IUnmanagedHost unmanagedInterface = null;

        /// <summary>
        ///		A new AppDomain has been created
        /// </summary>
        public override void InitializeNewDomain(AppDomainSetup appDomainInfo)
        {
            // let the unmanaged host know about us
            InitializationFlags = AppDomainManagerInitializationOptions.RegisterWithHost;

            return;
        }

        public int CreateAppDomain(string name)
        {
            PermissionSet permissions = new PermissionSet(PermissionState.None);
            permissions.AddPermission(new SecurityPermission(PermissionState.Unrestricted));
            permissions.AddPermission(new UIPermission(PermissionState.Unrestricted));

            return AppDomain.CreateDomain(
                name,
                AppDomain.CurrentDomain.Evidence,
                AppDomain.CurrentDomain.SetupInformation,
                permissions,
                CreateStrongName(Assembly.GetExecutingAssembly())).Id;
        }

        public void Dispose()
        {
            return;
        }

        public void SetUnmanagedHost(IUnmanagedHost unmanagedHost)
        {
            Debug.Assert(unmanagedHost != null, "Attempt to set null unmanaged host");
            Debug.Assert(this.unmanagedInterface == null, "Attempt to reset unmanaged host");

            this.unmanagedInterface = unmanagedHost;
            return;
        }

        public void Write(string message)
        {
            Trace.WriteLine(message);
            Console.WriteLine(message);
            return;
        }

        public Int64 Run(Int64 path)
        {
            return (path * 3);
            //new FileIOPermission(PermissionState.Unrestricted).Assert();
            //string fullPath = Path.Combine(
            //    Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location),
            //    path);
            //CodeAccessPermission.RevertAssert();

            //new FileIOPermission(
            //    FileIOPermissionAccess.Read | FileIOPermissionAccess.PathDiscovery,
            //    fullPath).Assert();
            //AppDomain.CurrentDomain.ExecuteAssembly(fullPath);
            //CodeAccessPermission.RevertAssert();
        }
    }
}
