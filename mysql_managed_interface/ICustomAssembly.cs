using System;
using System.Collections.Generic;
using System.Text;

namespace MySQLHostManager
{
    public interface ICustomAssembly
    {
        Int64 RunInteger(Int64 value);
        Int64 RunIntegers(Int64[] values);


        double RunReal(double value);
        double RunReals(double[] values);


        string RunString(string value);
        string RunStrings(string[] values);
    }
}
