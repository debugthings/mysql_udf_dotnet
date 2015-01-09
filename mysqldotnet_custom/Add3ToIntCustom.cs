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

            return (long)(value + 3);
        }

        public long RunIntegers(long[] values)
        {
            long retVal = 0;
            foreach (var item in values)
            {
                retVal += (item + 3);
            }
            return retVal;
        }

        public double RunReal(double value)
        {
            return value + 3.0;
        }

        public double RunReals(double[] values)
        {
            double retVal = 0;
            foreach (var item in values)
            {
                retVal += (item + 3);
            }
            return retVal;
        }

        public string RunString(string value)
        {
            return value + " SIMPLE EXAMPLE";
        }

        public string RunStrings(string[] values)
        {
            StringBuilder sb = new StringBuilder();
            foreach (var item in values)
            {
                sb.Append(string.Format("{0} SIMPLE EXAMPLE ", item));
            }
            return sb.ToString();
        }


    }
}
