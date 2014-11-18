mysql_udf_dotnet
================
A .NET Hosting API integration for MySQL using a User Developed Function. In other words, you can execute .NET code inside of MySQL.

#Overview
Large systems like SQL server give the DBAs and developers a lot of options for storing and managing data. One of those options is to install custom assemblies and run them inside of SQL server. This is a pet project to learn more about the .NET Hosting API.

I am commited to making the code work well for 90% of the use cases out there. For a while though, it will be buggy and ugly to look at. And getting it to work might be a chore for some.

#How to compile
Right now this compiles fine on my machine :smile:. But if you'd like to attempt a compile you should check out my [blog post](http://www.debugthings.com/2014/11/11/extending-mysql-server-part1/) that explains how to compile the MySQL source tree from scratch. But here is a short rundown. I am working on making a CMake package that will do all of this for us and generate the proper DLLs and assemblies.

##Prerequisites
1. CMake
2. Bison
3. Visual Studio 2013
 - If you use express you need both C++ and C# to compile the libraries.

##Steps
1. Download Source
2. Install Tools
3. Run CMAKE commands found [here](http://dev.mysql.com/doc/internals/en/cmake-howto-detailed.html)
4. Follow [this tutorial](http://dev.mysql.com/doc/refman/5.5/en/udf-compiling.html)
5. Download this repo
6. Change the location of the MySQL includes
7. Compile (hopefully)

#How it works
In short, it leverages the MySQL UDF framework that has been around forever. It also injects the .NET Hosting API, which has also been around forever and is somewhat documented. The **clr_host** project houses all of the .NET Hosting API calls; this is a static library. The **mysql_managed_interface** project contains the definitions for the IManagedHost and IUnmanagedHost. The **mysql_udf** project contains the base udf calls.

When you load a UDF into MySQL you give it the name of your function with a very specific format <name>_int, <name>_string, etc. It then looks for a few other exported function definitions that end with _init and _deinit.

In this code we initialize the CClrHost class in a global variable (eek!) and that is what gives us control over loading an AppDomain manager. Once we've loaded the CLR and set up our hosting parameters, our CClrHost will bind to the framework(s) and make a call into the managed host to set up our default application domain.

From here our AppDomain can execute code inside of this assembly or load other assemblies and execute that code as well. A simplified diagram is below. You'll notice that .NET covers only half of the code blocks. That's because some of the code executes in MySQL and some executes in the context of the CLR. However, to be clear, ALL code runs inside the MySQL process, but the threads will transition into managed.

![Block Diagram](http://www.debugthings.com/mysql/BlockDiagram.png)
