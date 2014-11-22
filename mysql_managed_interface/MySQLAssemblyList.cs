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
    }
}
