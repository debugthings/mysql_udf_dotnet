/*
  MySQL UDF .NET Install script

  This will create the functions listed in the README.txt to help you get started.
*/

DROP FUNCTION IF EXISTS mysqldotnet_real;
DROP FUNCTION IF EXISTS mysqldotnet_int;
DROP FUNCTION IF EXISTS mysqldotnet_string;

CREATE FUNCTION mysqldotnet_real RETURNS REAL SONAME "MySQLDotNet.dll";
CREATE FUNCTION mysqldotnet_int RETURNS INTEGER SONAME "MySQLDotNet.dll";
CREATE FUNCTION mysqldotnet_string RETURNS STRING SONAME "MySQLDotNet.dll";

CREATE SCHEMA dotnet_functions;

DROP FUNCTION IF EXISTS dotnet_functions.mysqldotnet_real;
CREATE FUNCTION dotnet_functions.simple_add3toint (input int) RETURNS int DETERMINISTIC
RETURN (SELECT mysqldotnet_int("MySQLCustomClass.CustomMySQLClass", input));

DROP FUNCTION IF EXISTS dotnet_functions.simple_add3toreal;
CREATE FUNCTION dotnet_functions.simple_add3toreal (input real) RETURNS real DETERMINISTIC
RETURN (SELECT mysqldotnet_real("MySQLCustomClass.CustomMySQLClass", input));

DROP FUNCTION IF EXISTS dotnet_functions.simple_addtostring;
CREATE FUNCTION dotnet_functions.simple_addtostring (input VARCHAR(255)) RETURNS TEXT
RETURN (SELECT mysqldotnet_string("MySQLCustomClass.CustomMySQLClass", input));

DROP FUNCTION IF EXISTS dotnet_functions.adv_isinradius;
CREATE FUNCTION dotnet_functions.adv_isinradius (LatCenter real, LongCenter real, LatPoint real,
  LongPoint real, radius real) RETURNS real
RETURN (SELECT mysqldotnet_string("MySQLCustomClass.PointsInRadius", "MULTI", LatCenter, LongCenter, LatPoint, LongPoint, radius));

DROP FUNCTION IF EXISTS dotnet_functions.adv_getwebpage;
CREATE FUNCTION dotnet_functions.adv_getwebpage (url VARCHAR(1000)) RETURNS TEXT
  RETURN (SELECT mysqldotnet_string("MySQLCustomClass.GetWebPage", url));


/*
  Please remember to grant EXECUTE permissions to non privileged users.
  http://dev.mysql.com/doc/refman/5.1/en/grant.html#grant-database-privileges

  GRANT EXECUTE ON PROCEDURE mydb.myproc TO 'someuser'@'somehost';

*/
