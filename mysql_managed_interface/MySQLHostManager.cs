using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Security;
using System.Security.Permissions;
using System.Security.Policy;

namespace mysql_managed_interface
{
    class MySQLHostManager : AppDomainManager, IManagedHost
    {
        private System.Collections.Generic.Dictionary<string, ICustomAssembly> functions = null;
        private string m_Assembly;
        private string m_Class;

        static void ADIDelegate(string[] args)
        {
            //var asm = AppDomain.CurrentDomain.Load(args[0]);
        }

        void CurrentDomain_AssemblyLoad(object sender, AssemblyLoadEventArgs args)
        {

            throw new NotImplementedException();
        }

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

        private IUnmanagedHost unmanagedInterface = null;

        /// <summary>
        ///		A new AppDomain has been created
        /// </summary>
        public override void InitializeNewDomain(AppDomainSetup appDomainInfo)
        {
            // let the unmanaged host know about us
            InitializationFlags = AppDomainManagerInitializationOptions.RegisterWithHost;


        }

        public string CreateAppDomain(string name)
        {
            return CreateAppDomain(name, "");
        }

        public string CreateAppDomain(string assemblyName, string className)
        {
            PermissionSet permissions = new PermissionSet(PermissionState.None);
            permissions.AddPermission(new SecurityPermission(SecurityPermissionFlag.Execution));

            AppDomainSetup ads = new AppDomainSetup();
            ads.AppDomainInitializer = ADIDelegate;
            ads.AppDomainInitializerArguments = new string[] { assemblyName, className };
            ads.ConfigurationFile = "mysqldotnet.config";
            ads.PrivateBinPath = @"lib\plugin";
            ads.PrivateBinPathProbe = "";

            string AppDomainName = DateTime.Now.ToFileTime().ToString();

            return string.Format("{0}||{1}", GetCLR(), AppDomain.CreateDomain(
                AppDomainName,
                AppDomain.CurrentDomain.Evidence,
                ads,
                permissions,
                CreateStrongName(Assembly.GetExecutingAssembly())).Id);
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
        }

        public string GetCLR()
        {
            return string.Format("v{0}", Environment.Version.ToString(3));
        }

        public long RunInteger(string functionName, long value)
        {
            return functions[functionName].RunInteger(value);
        }

        public long RunIntegers(string functionName, long[] values)
        {
            return functions[functionName].RunIntegers(values);
        }
        public double RunReal(string functionName, double value)
        {
            return functions[functionName].RunReal(value);
        }

        public double RunReals(string functionName, double[] values)
        {
            return functions[functionName].RunReals(values);
        }

        public string RunString(string functionName, string value)
        {
            return functions[functionName].RunString(value);
        }

        public string RunStrings(string functionName, string[] values)
        {
            return functions[functionName].RunStrings(values);
        }
    }
}
