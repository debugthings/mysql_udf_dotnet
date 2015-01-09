using System;
using System.Collections.Generic;
using System.Text;
using System.Configuration;

namespace MySQLHostManager
{

    public class MySQLAssemblyList : ConfigurationSection
    {
        public MySQLAssemblyList()
        {
        }

        [ConfigurationProperty("permissionsets")]
        [ConfigurationCollection(typeof(MySQLPermissionSets), AddItemName = "permissionset")]
        public MySQLPermissionSets permissionsetscollection
        {
            get
            {
                return (MySQLPermissionSets)base["permissionsets"];
            }
        }

        [ConfigurationProperty("assemblies", IsDefaultCollection = false)]
        [ConfigurationCollection(typeof(MySQLAssemblies), AddItemName = "assembly")]
        public MySQLAssemblies assemblies
        {
            get
            {
                return (MySQLAssemblies)base["assemblies"];
            }
        }

        [ConfigurationProperty("appDomainCleanup")]
        public MySQLAppDomainCleanup appDomainCleanup
        {
            get
            {
                return (MySQLAppDomainCleanup)base["appDomainCleanup"];
            }
        }

         [ConfigurationProperty("applicationDefaults")]
        public MySQLDefaultSettings applicationDefaults
        {
            get
            {
                return (MySQLDefaultSettings)base["applicationDefaults"];
            }
        }
    }

    public class MySQLAssemblies : ConfigurationElementCollection
    {

        protected override ConfigurationElement CreateNewElement()
        {
            return new MySQLAsembly();
        }

        protected override object GetElementKey(ConfigurationElement element)
        {
            return ((MySQLAsembly)element).name;
        }

        public void Add(MySQLAsembly item)
        {
            base.BaseAdd(item);
        }

        new public MySQLAsembly this[string Name]
        {
            get
            {
                return (MySQLAsembly)BaseGet(Name);
            }
        }
    }

    public class MySQLAsembly : ConfigurationElement
    {
        [ConfigurationProperty("name")]
        public string name
        {
            get
            {
                return (string)this["name"];
            }
            set
            {
                this["name"] = value;
            }
        }

        [ConfigurationProperty("fullname")]
        public string fullname
        {
            get
            {
                return (string)this["fullname"];
            }
            set
            {
                this["fullname"] = value;
            }
        }

        [ConfigurationProperty("clrversion")]
        public string clrversion
        {
            get
            {
                return (string)this["clrversion"];
            }
            set
            {
                this["clrversion"] = value;
            }
        }
        [ConfigurationProperty("permissions")]
        public string permissions
        {
            get
            {
                return (string)this["permissions"];
            }
            set
            {
                this["permissions"] = value;
            }
        }

        [ConfigurationProperty("lifetime", DefaultValue = "00:10:00")]
        public TimeSpan lifetime
        {
            get
            {
                return (TimeSpan)this["lifetime"];
            }
            set
            {
                this["lifetime"] = value;
            }
        }
    }

    public class MySQLPermissionSets : ConfigurationElementCollection
    {

        protected override ConfigurationElement CreateNewElement()
        {
            return new MySQLPermissionSet();
        }

        protected override object GetElementKey(ConfigurationElement element)
        {
            return ((MySQLPermissionSet)element).Name;
        }

        public void Add(MySQLPermissionSet item)
        {
            base.BaseAdd(item);
        }

        new public MySQLPermissionSet this[string Name]
        {
            get
            {
                return (MySQLPermissionSet)BaseGet(Name);
            }
        }
    }

    public class MySQLPermissionSet : ConfigurationElement
    {

        [ConfigurationProperty("name", IsKey = true, IsRequired = true)]
        public string Name
        {
            get
            {
                return (string)this["name"];
            }
            set
            {
                this["name"] = value;
            }
        }

        [ConfigurationProperty("permissions", IsDefaultCollection = false)]
        [ConfigurationCollection(typeof(MySQLPermissions), AddItemName = "add")]
        public MySQLPermissions permissionscollection
        {
            get
            {
                return (MySQLPermissions)base["permissions"];
            }
        }

    }

    public class MySQLPermissions : ConfigurationElementCollection
    {

        protected override ConfigurationElement CreateNewElement()
        {
            return new MySQLPermission();
        }

        protected override object GetElementKey(ConfigurationElement element)
        {
            return ((MySQLPermission)element).Name;
        }

        public void Add(MySQLPermission item)
        {
            base.BaseAdd(item);
        }

        new public MySQLPermission this[string Name]
        {
            get
            {
                return (MySQLPermission)BaseGet(Name);
            }
        }

    }

    public class MySQLPermission : ConfigurationElement
    {
        [ConfigurationProperty("name")]
        public string Name
        {
            get
            {
                return (string)this["name"];
            }
            set
            {
                this["name"] = value;
            }
        }
    }

    public class MySQLAppDomainCleanup : ConfigurationElement
    {
        [ConfigurationProperty("interval", DefaultValue = "00:01:00")]
        public TimeSpan interval
        {
            get
            {
                return (TimeSpan)this["interval"];
            }
            set
            {
                this["interval"] = value;
            }
        }

        [ConfigurationProperty("forcedInterval", DefaultValue = "06:00:00")]
        public TimeSpan forcedInterval
        {
            get
            {
                return (TimeSpan)this["forcedInterval"];
            }
            set
            {
                this["forcedInterval"] = value;
            }
        }

        [ConfigurationProperty("defaultLifetime", DefaultValue = "00:10:00")]
        public TimeSpan defaultLifetime
        {
            get
            {
                return (TimeSpan)this["defaultLifetime"];
            }
            set
            {
                this["defaultLifetime"] = value;
            }
        }

        [ConfigurationProperty("maxAllowableDomains", DefaultValue = "1000")]
        [IntegerValidator(MinValue = 20, MaxValue = int.MaxValue)] // Why impose a MinValue? This keeps people from creating a slow environment
        public int maxAllowableDomains
        {
            get
            {
                return (int)this["maxAllowableDomains"];
            }
            set
            {
                this["maxAllowableDomains"] = value;
            }
        }

    }

    public class MySQLDefaultSettings : ConfigurationElement
    {
        [ConfigurationProperty("codepage", DefaultValue = 1252)]
        public int codepage
        {
            get
            {
                return (int)this["codepage"];
            }
            set
            {
                this["codepage"] = value;
            }
        }

        [ConfigurationProperty("multikeyword", DefaultValue = "MULTI")]
        public string multikeyword
        {
            get
            {
                return (string)this["multikeyword"];
            }
            set
            {
                this["multikeyword"] = value;
            }
        }

    }
}
