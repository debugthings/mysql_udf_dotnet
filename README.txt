===============================================================================
MySQL .NET UDF - A UDF plugin that allows you to use .NET code to execute 
custom code just as you were to write a native UDF.

Written by James Davis.
www.debugthings.com
===============================================================================

INTRODUCTION
------------
While looking around at the plugins available for MySQL I noticed there wasn't 
much in the way of things for Windows. Even more scare were any items relating
to .NET.

I wrote a wrapper around the .NET Hosting APIs to allow a developer to code in
.NET and avoid all of the issues surrounding memory management, threading, and
isolation of potentially harmful code.

You now have the power of .NET at your control and can write some very crazy
albeit possibly not practical custom functions. Some examples would include
using WebClient to download webpages directly into a database without having to 
write an application to do so.

QUICKSTART
----------
If you want to jump in using the included examples here is what you need to do.

* Rename mysqld.exe.config_full to mysqld.exe.config
	- Delete old config file
* Open your favorite MySQL client and execute install_samples.sql

Execute one of the following stored procedures. Descriptions included.

simple_add3toint(int) -- Adds 3 to the input number

simple_add3toreal(real) -- Adds 3 to the input number

simple_addtostring(string) -- Adds "SIMPLE EXAMPLE" to the end of the input

adv_isinradius(LatCenter, LongCenter, LatPoint, LongPoint, radius) -- Calculates 
to see if the point is inside of the specified radius from the center.

adv_getwebpage(webpage) -- Uses System.WebClient to pull the raw HTML back from 
the URL in the function.


WHY .NET?
---------
The choice for me was simple. I work in .NET all the time.

WHY MYSQL?
----------
I have always had a soft spot for MySQL. It was my first RDBMS I used to create
a few websites. I started using it in 2000 and dabbled with it off and on for 
the past 14 years.

MySQL fell by the wayside once I got into a corporate environment and started 
using Microsoft SQL(MSSQL) and Oracle. However, one day I started thinking 
about ways I could use external code in MySQL similar to MSSQL.

The natural fit was to use .NET as that is what MSSQL used.

-------------------------------------------------------------------------------
INSTALLATION
-------------------------------------------------------------------------------
This README is shown after the installation has completed. But should also be 
found in the "MySQL .NET UDF" installation location. Below are steps listed
to do a manual installation if you happen to build your own copy or do not
use the installer.

BASE PLUGIN INSTALL (MANUAL)
----------------------------
	* Extract all files into a final destination folder
	* Install DOTNET20\MySQLHostManager.dll into the GAC
		- You can do this by dragging/dropping the files into %WINDIR%\assembly\ 
		or using %WINDIR%\Microsoft.NET\Framework\v2.0.50727\gacutil.exe
	* Install DOTNET40\MySQLHostManager.dll into the GAC
		- There is only one option. You can do this by using 
		%WINDIR%\Microsoft.NET\Framework\v2.0.50727\gacutil.exe
	* Copy bin\mysqld.exe.config to %MYSQLHOME%\bin
	* Copy plugin\MySQLDotNet.dll to %MYSQLHOME%\lib\plugin
	* Open your favorite MySQL client and execute sql\install.sql 

.NET CUSTOM PLUGIN INSTALL (MANUAL)
-----------------------------------
	* Copy your custom plugin dll to %MYSQLHOME%\lib\plugin
		- Optionally you may copy it to %MYSQLHOME%\lib\plugin\<DLLNAME>
		- See section on custom assemblies
	* Edit %MYSQLHOME%\bin\mysqld.exe.config to include your assembly
		- Examples are inside of the config file


EXECUTING CUSTOM CODE (MINI VERSION)
------------------------------------
If the install went well 

-------------------------------------------------------------------------------
HOW TO USE
-------------------------------------------------------------------------------
Once both the base plugin and your custom plugin are installed you need to tell
MySQL what assemblies it is allowed to load. To do this you need to edit the
.config file to include a new config section. Once the config section is added
you need to tell the application the name of your assembly to load.

This assembly will get picked up from the GAC or from lib\plugin or 
lib\plugin\ASSEMBLYNAME.

CUSTOM SECTION DECLARATION
--------------------------
<configSections>
<section name ="mysqlassemblies" type="MySQLHostManager.MySQLAssemblyList, 
            MySQLHostManager, Version=2.0.0.0, Culture=neutral, 
			PublicKeyToken=71c4a5d4270bd29c"/>
</configSections>

This is a standard custom section declaration. You can find out more
information from MSDN.

ASSEMBLY CONFIG DEFINITIION
---------------------------
<mysqlassemblies>
<assemblies>
    <assembly name="MySQLCustomClass.CustomMySQLClass"
			fullname ="MySQLCustomClass, 
            Version=1.0.0.0, PublicKeyToken=a55d172c54d273f4" 
			clrversion="4.0" lifetime="02:00:00"/>
</assemblies>
</mysqlassemblies>

This is a custom section that defines the assembly to be loaded. You must
sign your assembly and use a (partial) strong name. This is a safety feature
and protects against rogue code.

ATTRIBUTES
----------
- name: The fully qualified name of the class that you wish to use
- fullname: The strong name of your assembly
- clrversion: The required version of the CLR. Uses default if not specified
- lifetime: How long the appdomains live for this assembly

APPDOMAIN CONFIG DEFINITIION
----------------------------
<mysqlassemblies>
<appDomainCleanup
      interval="0.00:05:00"
      forcedInterval="1.00:00:00" />
</mysqlassemblies>

All of the domains are entered into a pool and reused. After the specified
lifetime the domains are released from the pool and set to "die." This custom
section controls intervals the removed domains are unloaded from the
application.

The only time domains are checked for life is when they are being pulled from
the pool and being released from active use. The forcedInterval makes sure that
upon the next unload check the system will iterate through all appdomains and
force the timeout check.

ATTRIBUTES
----------
- interval: The timespan between checking to unload.
- forcedInterval: All domains are scanned and cleared if they meet the criteria

PERMISSION SET CONFIG DEFINITION
--------------------------------
<mysqlassemblies>
	<permissionsets>
		<permissionset name="MySQLPartial">
		<permissions>
			<add name="FileIOPermission" />
		</permissions>
		</permissionset>
		<!---->
	</permissionsets>
<mysqlassemblies>

In order to secure the host and to secure the code the AppDomain manager
enforces Code Access Security. In order to run specific methods inside of
MySQL we need to grant specific code access.

These names come from the standard code access security classes inside of 
.NET. Check MSDN for a description of each class. Some common permissions:

- FileIOPermission
- WebPermission
- RegistryPermission
- EventLogPermission

