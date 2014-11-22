using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security;
using System.Security.Permissions;
using System.Security.Policy;

namespace MySQLHostManager
{
    class MySQLHostManager : AppDomainManager, IManagedHost
    {
        private System.Collections.Generic.Dictionary<string, ICustomAssembly> functions = null;
        private System.Collections.Generic.Dictionary<string, AppDomain> activeAppDomains = new System.Collections.Generic.Dictionary<string, AppDomain>();
        private string currentAppDomainName = String.Empty;
        GCHandle gchand;
        static void ADIDelegate(string[] args)
        {
            var asm = AppDomain.CurrentDomain.Load(args[0]);

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
            if (AppDomain.CurrentDomain.IsDefaultAppDomain())
            {
                InitializationFlags = AppDomainManagerInitializationOptions.RegisterWithHost;
            }
        }

        public IManagedHost CreateAppDomain(string typeName)
        {
            var section = System.Configuration.ConfigurationManager.GetSection("mysqlassemblies") as MySQLAssemblyList;
            var assemblyName = typeName.Split('.')[0];
            var className = typeName.Split('.')[1];
            var obj = section.assemblies[assemblyName];

            PermissionSet permissions = new PermissionSet(PermissionState.None);
            permissions.AddPermission(new SecurityPermission(SecurityPermissionFlag.Execution));

            AppDomainSetup ads = new AppDomainSetup();
            ads.AppDomainInitializer = ADIDelegate;
            ads.AppDomainInitializerArguments = new string[] { assemblyName, className };
            ads.ConfigurationFile = "mysqldotnet.config";
            ads.ApplicationBase = string.Format("{0}..\\", AppDomain.CurrentDomain.SetupInformation.ApplicationBase);
            ads.PrivateBinPath = "RelWithDebInfo;lib\\plugin";

            string AppDomainName = DateTime.Now.ToFileTime().ToString();

            var appdomain = AppDomain.CreateDomain(
                AppDomainName,
                AppDomain.CurrentDomain.Evidence,
                ads,
                permissions,
                CreateStrongName(Assembly.GetExecutingAssembly()));

            activeAppDomains.Add(AppDomainName, appdomain);

            return (IManagedHost)appdomain.DomainManager;
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

        private void InitFunctions(string functionName)
        {
            if (functions == null)
            {
                functions = new System.Collections.Generic.Dictionary<string, ICustomAssembly>();
            }
            if (!functions.ContainsKey(functionName))
            {
                foreach (var item in AppDomain.CurrentDomain.GetAssemblies())
                {
                    var typ = item.GetType(functionName);
                    if (typ != null && typ.GetInterface("MySQLHostManager.ICustomAssembly") == typeof(ICustomAssembly))
                    {
                        functions.Add(functionName, (ICustomAssembly)item.CreateInstance(functionName));
                    }
                }
            }
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
            InitFunctions(functionName);
            return functions[functionName].RunInteger(value);
        }

        public long RunIntegers(string functionName, long[] values)
        {
            InitFunctions(functionName);
            return functions[functionName].RunIntegers(values);
        }
        public double RunReal(string functionName, double value)
        {
            InitFunctions(functionName);
            return functions[functionName].RunReal(value);
        }

        public double RunReals(string functionName, double[] values)
        {
            InitFunctions(functionName);
            return functions[functionName].RunReals(values);
        }

        public string RunString(string functionName, string value)
        {
            InitFunctions(functionName);
            return functions[functionName].RunString(value);
        }

        public string RunStrings(string functionName, string[] values)
        {
            InitFunctions(functionName);
            return functions[functionName].RunStrings(values);
        }


        public string GetAssemblyCLRVersion(string AssemblyName)
        {
            return System.Configuration.ConfigurationManager.AppSettings[AssemblyName];
        }

        public string GetAppDomainName
        {
            get
            {

                if (string.IsNullOrEmpty(currentAppDomainName))
                {
                    currentAppDomainName = AppDomain.CurrentDomain.FriendlyName;
                    gchand = GCHandle.Alloc(currentAppDomainName, GCHandleType.Pinned);
                }
                return currentAppDomainName;
            }
        }

        public bool Unload(string FriendlyName)
        {
            try
            {
                if (activeAppDomains.ContainsKey(FriendlyName))
                {
                    AppDomain.Unload(activeAppDomains[FriendlyName]);
                }
                else
                {
                    return false;
                }
                
            }
            catch (Exception)
            {
                return false;
            }
            return true;
        }
    }
}
