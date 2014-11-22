using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MySQLCustomClass
{
    public class CustomMySQLClass : MySQLHostManager.ICustomAssembly
    {
        public long RunInteger(long value)
        {

            return (long)(value * Math.PI) + 10;
        }

        public long RunIntegers(long[] values)
        {
            return 0L;
        }

        public double RunReal(double value)
        {
            return 0.0;
        }

        public double RunReals(double[] values)
        {
            return 0.0;
        }

        public string RunString(string value)
        {
            return string.Empty;
        }

        public string RunStrings(string[] values)
        {
            return string.Empty;
        }


    }
}
