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
        private System.Collections.Generic.Dictionary<string, AppDomain> activeAppDomains = null;
        private string currentAppDomainName = String.Empty;
        GCHandle gchand;
        /// <summary>
        /// Loads the assembly based on the full name. This can obviously be a partialy qualified name.
        /// </summary>
        /// <param name="args">The parameters passed in from our CreateDomain() method.</param>
        static void ADIDelegate(string[] args)
        {
            var asm = AppDomain.CurrentDomain.Load(args[0]);

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

        public static StrongName CreateStrongNameFromString(string assembly)
        {

            if (assembly == null)
                throw new ArgumentNullException("assembly");

            AssemblyName assemblyName = new AssemblyName(assembly);
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
        /// <summary>
        /// Gets the named permission set from the permissionset collection defined in the configuration file.
        /// </summary>
        /// <param name="typeName">The name of the type defined in the configuration file.</param>
        /// <returns>A permission set based on the collection of permissions in the defined permission set.</returns>
        public PermissionSet GetAssemblyPermissions(MySQLAsembly typeName)
        {
            PermissionSet permissions = new PermissionSet(PermissionState.None);
            permissions.AddPermission(new SecurityPermission(SecurityPermissionFlag.Execution));

            if (!string.IsNullOrEmpty(typeName.permissions))
            {
                if (typeName.permissions.Equals("fulltrust", StringComparison.InvariantCultureIgnoreCase))
                {
                    // override the default permission set with a full trust permission set.
                    permissions = new PermissionSet(PermissionState.Unrestricted);
                }
                else
                {
                    var section2 = System.Configuration.ConfigurationManager.GetSection("mysqlassemblies") as MySQLAssemblyList;
                    var permlists = section2.permissionsetscollection;
                    var permlist = permlists[typeName.permissions];

                    // For now we're adding all of the permissions out of the box for System, System.Configuration, System.Data. and System.Xml
                    // This is heavy handed, but aides in beta testing. By the end we should have pared this down to a handful of useful permissions
                    // ie. UI is not needed.
                    foreach (MySQLPermission permission in permlist.permissionscollection)
                    {
                        switch (permission.Name)
                        {
                            case "AspNetHostingPermission": permissions.AddPermission(new System.Web.AspNetHostingPermission(PermissionState.Unrestricted)); break;
                            //case "Collaboration": permissions.AddPermission(new System.Net.PeerToPeer.Collaboration.PeerCollaborationPermission(PermissionState.Unrestricted));
                            //    break;
                            case "ConfigurationPermission": permissions.AddPermission(new System.Configuration.ConfigurationPermission(PermissionState.Unrestricted));
                                break;
                            //case "DataProtectionPermission": permissions.AddPermission(new System.Security.Permissions.DataProtectionPermission(PermissionState.Unrestricted));
                            //    break;
                            case "OdbcPermission": permissions.AddPermission(new System.Data.Odbc.OdbcPermission(PermissionState.Unrestricted));
                                break;
                            case "OleDbPermission": permissions.AddPermission(new System.Data.OleDb.OleDbPermission(PermissionState.Unrestricted));
                                break;
                            case "SqlClientPermission": permissions.AddPermission(new System.Data.SqlClient.SqlClientPermission(PermissionState.Unrestricted));
                                break;
                            //case "DistributedTransactionPermission": permissions.AddPermission(new System.Transactions.DistributedTransactionPermission(PermissionState.Unrestricted));
                            //    break;
                            case "EnvironmentPermission": permissions.AddPermission(new System.Security.Permissions.EnvironmentPermission(PermissionState.Unrestricted));
                                break;
                            case "FileDialogPermission": permissions.AddPermission(new System.Security.Permissions.FileDialogPermission(PermissionState.Unrestricted));
                                break;
                            case "FileIOPermission": permissions.AddPermission(new System.Security.Permissions.FileIOPermission(PermissionState.Unrestricted));
                                break;
                            case "GacIdentityPermission": permissions.AddPermission(new System.Security.Permissions.GacIdentityPermission(PermissionState.Unrestricted));
                                break;
                            case "IsolatedStorageFilePermission": permissions.AddPermission(new System.Security.Permissions.IsolatedStorageFilePermission(PermissionState.Unrestricted));
                                break;
                            case "KeyContainerPermission": permissions.AddPermission(new System.Security.Permissions.KeyContainerPermission(PermissionState.Unrestricted));
                                break;
                            //case "MediaPermission": permissions.AddPermission(new System.Security.Permissions.MediaPermission(PermissionState.Unrestricted));
                            //     break;
                            //case "MessageQueuePermission": permissions.AddPermission(new System.Messaging.MessageQueuePermission(PermissionState.Unrestricted));
                            //     break;
                            case "NetworkInformationPermission": permissions.AddPermission(new System.Net.NetworkInformation.NetworkInformationPermission(PermissionState.Unrestricted));
                                break;
                            //case "OraclePermission": permissions.AddPermission(new System.Data.OracleClient.OraclePermission(PermissionState.Unrestricted));
                            //     break;
                            //case "PeerCollaborationPermission": permissions.AddPermission(new System.Net.PeerToPeer.Collaboration.PeerCollaborationPermission(PermissionState.Unrestricted));
                            //    break;
                            //case "PnrpPermission": permissions.AddPermission(new System.Net.PeerToPeer.PnrpPermission(PermissionState.Unrestricted));
                            //     break;
                            //case "PrintingPermission": permissions.AddPermission(new System.Drawing.Printing.PrintingPermission(PermissionState.Unrestricted));
                            //     break;
                            case "PublisherIdentityPermission": permissions.AddPermission(new System.Security.Permissions.PublisherIdentityPermission(PermissionState.Unrestricted));
                                break;
                            case "ReflectionPermission": permissions.AddPermission(new System.Security.Permissions.ReflectionPermission(PermissionState.Unrestricted));
                                break;
                            case "RegistryPermission": permissions.AddPermission(new System.Security.Permissions.RegistryPermission(PermissionState.Unrestricted));
                                break;
                            case "EventLogPermission": permissions.AddPermission(new System.Diagnostics.EventLogPermission(PermissionState.Unrestricted));
                                break;
                            case "PerformanceCounterPermission": permissions.AddPermission(new System.Diagnostics.PerformanceCounterPermission(PermissionState.Unrestricted));
                                break;
                            //case "DirectoryServicesPermission": permissions.AddPermission(new System.DirectoryServices.DirectoryServicesPermission(PermissionState.Unrestricted));
                            //     break;
                            //case "ServiceControllerPermission": permissions.AddPermission(new System.ServiceProcess.ServiceControllerPermission(PermissionState.Unrestricted));
                            //     break;
                            case "SecurityPermission": permissions.AddPermission(new System.Security.Permissions.SecurityPermission(PermissionState.Unrestricted));
                                break;
                            case "SiteIdentityPermission": permissions.AddPermission(new System.Security.Permissions.SiteIdentityPermission(PermissionState.Unrestricted));
                                break;
                            case "SmtpPermission": permissions.AddPermission(new System.Net.Mail.SmtpPermission(PermissionState.Unrestricted));
                                break;
                            case "SocketPermission": permissions.AddPermission(new System.Net.SocketPermission(PermissionState.Unrestricted));
                                break;
                            case "StorePermission": permissions.AddPermission(new System.Security.Permissions.StorePermission(PermissionState.Unrestricted));
                                break;
                            case "StrongNameIdentityPermission": permissions.AddPermission(new System.Security.Permissions.StrongNameIdentityPermission(PermissionState.Unrestricted));
                                break;
                            //case "TypeDescriptorPermission": permissions.AddPermission(new System.Security.Permissions.TypeDescriptorPermission(PermissionState.Unrestricted));
                            //     break;
                            case "UIPermission": permissions.AddPermission(new System.Security.Permissions.UIPermission(PermissionState.Unrestricted));
                                break;
                            case "UrlIdentityPermission": permissions.AddPermission(new System.Security.Permissions.UrlIdentityPermission(PermissionState.Unrestricted));
                                break;
                            //case "WebBrowserPermission": permissions.AddPermission(new System.Security.Permissions.WebBrowserPermission(PermissionState.Unrestricted));
                            //     break;
                            case "WebPermission": permissions.AddPermission(new System.Net.WebPermission(PermissionState.Unrestricted));
                                break;
                            //case "XamlLoadPermission": permissions.AddPermission(new System.Xaml.Permissions.XamlLoadPermission(PermissionState.Unrestricted));
                            //     break;

                            default:
                                break;
                        }
                    }
                }

            }

            return permissions;
        }

        public IManagedHost CreateAppDomain(string typeName)
        {

            if (activeAppDomains == null)
            {
                activeAppDomains = new System.Collections.Generic.Dictionary<string, AppDomain>();
            }
            // Get the assembly from the config file.
            var section = System.Configuration.ConfigurationManager.GetSection("mysqlassemblies") as MySQLAssemblyList;
            var assemblyName = typeName.Split('.')[0];
            var className = typeName.Split('.')[1];
            var obj = section.assemblies[typeName];

            if (obj != null)
            {
                var permissions = GetAssemblyPermissions(obj);

                AppDomainSetup ads = new AppDomainSetup();
                ads.AppDomainInitializer = ADIDelegate;
                ads.AppDomainInitializerArguments = new string[] { obj.fullname, className };
                ads.ConfigurationFile = "mysqldotnet.config";
                ads.ApplicationBase = string.Format("{0}..\\", AppDomain.CurrentDomain.SetupInformation.ApplicationBase);
                ads.PrivateBinPath = "RelWithDebInfo;lib\\plugin"; //TODO make sure this is correct for GA versions, not the compiled versions
                ads.ShadowCopyFiles = "true";

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

            // Why throw an exception instead of returning NULL? Well, to be honest it is a shortcut. The fact that we don't have the assembly is
            // not truly an exception, but the fact we can't load the AppDomain is.
            throw new NullReferenceException("Assembly used in MySQL query does not exist. Please check your spelling and make sure it is defined in the config file.");
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

        public string GetCLR()
        {
            return string.Format("v{0}", Environment.Version.ToString(2));
        }

        public string GetAssemblyCLRVersion(string AssemblyName)
        {
            // Get the assembly from the config file.
            var section = System.Configuration.ConfigurationManager.GetSection("mysqlassemblies") as MySQLAssemblyList;
            var assemblyName = AssemblyName.Split('.')[0];
            var className = AssemblyName.Split('.')[1];
            var obj = section.assemblies[AssemblyName];
            if (obj.clrversion.StartsWith("v", StringComparison.InvariantCultureIgnoreCase))
            {
                return obj.clrversion;
            }
            return "v" + obj.clrversion;
        }

        public string GetAppDomainName
        {
            get
            {
                if (string.IsNullOrEmpty(currentAppDomainName))
                {
                    currentAppDomainName = AppDomain.CurrentDomain.FriendlyName;
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


        #region MySQL mapped functions
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

        #endregion
    }
}
