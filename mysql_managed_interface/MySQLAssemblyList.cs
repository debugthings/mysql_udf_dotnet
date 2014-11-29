using System;
using System.Collections.Generic;
using System.Text;
using System.Configuration;

namespace MySQLHostManager
{

    class MySQLAssemblyList : ConfigurationSection
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
    }

    class MySQLAssemblies : ConfigurationElementCollection
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

    class MySQLAsembly : ConfigurationElement
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
    }

    class MySQLPermissionSets : ConfigurationElementCollection
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

    class MySQLPermissionSet : ConfigurationElement
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
        [ConfigurationCollection(typeof(MySQLPermissions), AddItemName="add")]
        public MySQLPermissions permissionscollection
        {
            get
            {
                return (MySQLPermissions)base["permissions"];
            }
        }
       
    }

    class MySQLPermissions : ConfigurationElementCollection
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

    class MySQLPermission : ConfigurationElement
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
}
