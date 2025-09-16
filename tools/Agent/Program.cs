
using System.Reflection;
using Task = System.Threading.Tasks.Task;
using Agent;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Diagnostics;
using static Agent.Executer;
using System;
#if DEBUG
using Agent.LocalTesting;
#endif

class Program
{
    // Used by rundll32
    // rundll32 win-x64\PortableApp.dll Open32 vid=cafe pid=1001 cwd=%APPDATA%
    [UnmanagedCallersOnly(EntryPoint = "Open32", CallConvs = new Type[] { typeof(CallConvStdcall) })]
    public static void Open32(IntPtr hwnd, IntPtr hinst, IntPtr lpszCmdLine, int nCmdShow)
    {
#pragma warning disable CS0168 // Variable is declared but never used
        try
        {
            var cmdLine = Marshal.PtrToStringAnsi(lpszCmdLine);
            if (!string.IsNullOrWhiteSpace(cmdLine))
            {
                Main(cmdLine.Split(" ")).Wait();
            }
        }
        catch (Exception ex)
        {
#if DEBUG
            File.AppendAllText("C:\\temp\\log.txt", ex.Message);
#endif
        }
#pragma warning restore CS0168 // Variable is declared but never used
    }

    // Used by anyone else who can call a DLL entrypoint
    [UnmanagedCallersOnly(EntryPoint = "Open")]
    public static void Open(IntPtr cmdLinePtr)
    {
#pragma warning disable CS0168 // Variable is declared but never used
        try
        {
            var cmdLine = Marshal.PtrToStringUni(cmdLinePtr);
            if (!string.IsNullOrWhiteSpace(cmdLine))
            {
                Main(cmdLine.Split(" ")).Wait();
            }
        }
        catch (Exception ex)
        {
#if DEBUG
            File.AppendAllText("C:\\temp\\log.txt", ex.Message);
#endif
        }
#pragma warning restore CS0168 // Variable is declared but never used
    }

#if DEBUG
    public
#else
    private
#endif
        static async Task Main(string[] args)
    {
        // Set the current working directory if we were given one on the cli
        args.Where(x => x.StartsWith("cwd=")).Select(x => x.Remove(0, 4)).ToList().ForEach(x => Environment.CurrentDirectory = Environment.ExpandEnvironmentVariables(x));

        using (Mutex mutex = new Mutex(true, Assembly.GetExecutingAssembly().FullName, out bool createdNew))
        {
            if (!createdNew)
            {
#if DEBUG
                Console.WriteLine("Already running, quitting");
#endif
                return;
            }

#if DEBUG
            if (args.Any(x => x == "tcp"))
            {
                await TcpServer.Start(7000);
            }
            else if (args.Any(x => x == "web"))
            {
                WebServer.Start(7000, Environment.CurrentDirectory);
            }
            else
            {
#endif
                var vid = args.Where(x => x.StartsWith("vid=")).Single();
                var pid = args.Where(x => x.StartsWith("pid=")).Single();
                vid = vid.Split("=")[1].ToUpperInvariant();
                pid = pid.Split("=")[1].ToUpperInvariant();

                SerialComms serial = new();
                while (true)
                {
                    try
                    {
                        serial.Start(vid, pid);
                    }
                    catch (DeviceNotFoundException)
                    {
                        await Task.Delay(TimeSpan.FromSeconds(1));
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex);
                        await Task.Delay(TimeSpan.FromSeconds(1));
                    }
                }
#if DEBUG
            }
#endif
        }
    }
}
