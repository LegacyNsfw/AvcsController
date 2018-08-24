using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PlxParser
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                Console.Clear();
                SerialPort port = OpenPort();
                Loop(port);
            }
            catch (Exception exception)
            {
                Console.WriteLine(exception.ToString());
                if (System.Diagnostics.Debugger.IsAttached)
                {
                    Console.ReadLine();
                }
            }            
        }

        private static SerialPort OpenPort()
        {
            Console.WriteLine("Opening port.");
            SerialPort port = new SerialPort("COM8", 19200, Parity.None, 8, StopBits.One);
            port.Open();
            Console.WriteLine("Port opened.");
            return port;
        }

        private static void Loop(SerialPort port)
        {
            int value;
            long iterations = 0;
            int state = 0;
            int temp = 0;
            while((value = port.ReadByte()) != (int)-1)
            {
                if(value == 0x80)
                {
                    Console.SetCursorPosition(0, 0);
                    state = 1;
                    continue;
                }

                if (value == 0x40)
                {
                    Write("END");
                    iterations++;
                    Console.WriteLine();
                    Write(iterations + " iterations.");
                    //Console.Clear();
                    state = 0;
                    continue;
                }

                switch (state)
                {
                    case 1:
                        temp = value << 6;
                        state = 2;
                        break;

                    case 2:
                        temp |= value;
                        Write("Address " + temp);
                        state = 3;
                        break;

                    case 3:
                        Write("Instance " + value);
                        state = 4;
                        break;

                    case 4:
                        temp = value << 6;
                        state = 5;
                        break;

                    case 5:
                        temp |= value;
                        Write("Data " + temp);
                        Write(string.Empty);
                        state = 1;
                        break;

                    default:
                        Write(string.Format("State {0} value {1,3:X}", state, value));
                        break;
                }

                // Write(value);
            }
        }

        private static void Write(string message)
        {
            Console.WriteLine(string.Format("{0,20}", message));
        }

        private static void Write(int value)
        {
            Console.WriteLine(string.Format("{0,20:X}", value));
        }

    }
}
