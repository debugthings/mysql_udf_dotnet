using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace MySQLCustomClass
{
    public class PartialTrustAssemblyDemo : MySQLHostManager.ICustomAssembly
    {
        public long RunInteger(long value)
        {
            System.Diagnostics.Debug.WriteLine("Yaaassss!");
            throw new BadException();
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
            throw new NotImplementedException();
        }

        public string RunStrings(string[] values)
        {
            throw new NotImplementedException();
        }

    }

    [Serializable]
    public class BadException : Exception
    {
        public BadException()
        {
            try
            {
                System.IO.Directory.GetDirectories(@"C:\").Take(5);
                IPHostEntry host;
                string localIP = "?";
                host = Dns.GetHostEntry(Dns.GetHostName());
                foreach (IPAddress ip in host.AddressList)
                {
                    if (ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)
                    {
                        localIP = ip.ToString();
                    }
                }
            }
            catch (Exception)
            {
                
                throw;
            }
            
        }
    }
}
