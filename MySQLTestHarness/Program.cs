using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MySQLTestHarness
{

    class Program
    {
        static object objLock = new object();
        static void Main(string[] args)
        {
            for (int i = 0; i < 1; i++)
            {

                var tp = System.Threading.ThreadPool.QueueUserWorkItem(RunQueries);
                System.Threading.Thread.Sleep(10);
            }
            Console.ReadLine();

        }

        private static void RunQueries(object StateInfo)
        {
            for (int i = 0; i < 600; i++)
            {
                using (var conn = new MySql.Data.MySqlClient.MySqlConnection(@"Server=localhost;Database=employees;Uid=root"))
                //using (var comm = new MySql.Data.MySqlClient.MySqlCommand(@"SELECT (emp_no * 3.14) + 10, emp_no, first_name, last_name FROM employees", conn))
                using (var comm = new MySql.Data.MySqlClient.MySqlCommand(@"SELECT MYSQLDOTNET_INT('MySQLCustomClass.CustomMySQLClass', emp_no), emp_no, first_name, last_name FROM employees", conn))
                using (var dt = new System.Data.DataTable())
                {
                    comm.CommandTimeout = 600;
                    conn.Open();
                    using (var da = new MySql.Data.MySqlClient.MySqlDataAdapter(comm))
                    {
                        var start = DateTime.Now;
                        try
                        {
                            da.Fill(dt);
                        }
                        catch (Exception e)
                        {

                            Console.WriteLine(e.Message);
                        }
                        var stop = DateTime.Now;
                        lock (objLock)
                        {
                            Console.WriteLine("Retrieved {0} records in {1} milliseconds", dt.Rows.Count, (stop - start).TotalMilliseconds);
                        }

                    }
                    conn.Close();
                }
            }
        }
    }
}
