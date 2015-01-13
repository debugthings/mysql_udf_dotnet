using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MySQLCustomClass
{
    public class GetWebPage : MySQLHostManager.ICustomAssembly
    {
        public long RunInteger(long value)
        {
            throw new NotImplementedException();
        }

        public long RunIntegers(long[] values)
        {
            throw new NotImplementedException();
        }

        public double RunReal(double value)
        {
            throw new NotImplementedException();
        }

        public double RunReals(double[] values)
        {
            throw new NotImplementedException();
        }

        public string RunString(string value)
        {
            using (var wc = new System.Net.WebClient())
            {
                var str = wc.DownloadString(value);
                return str;
            }
        }

        public string RunStrings(string[] values)
        {
            throw new NotImplementedException();
        }
    }
}
