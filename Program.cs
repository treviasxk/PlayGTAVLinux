using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading;

internal class Program
{
    // Locations files
    static string filePlayGTAV = "PlayGTAV";
    static string fileGTAV = "GTA5";
    static string fileGTAVEnhanced = "GTA5_Enhanced";
    static string fileServiceLog = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData), @"Rockstar Games\Launcher\") + "service_log.txt";

    // CommandLines addicionals
    static string commandLines = "";
    static int lastLine = 0;

    static bool byPass = false, isUpdating = false, IsRunning = false, CheckClose = false;


    [DllImport("kernel32.dll")]
    static extern IntPtr GetConsoleWindow();

    [DllImport("user32.dll")]
    static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

    const int SW_HIDE = 0;

    private static void Main(string[] args)
    {
        foreach (var line in Environment.GetCommandLineArgs().Skip(1))
            commandLines += " " + line;

        IntPtr handle = GetConsoleWindow();
        if(!commandLines.Contains("-showlogs"))
            ShowWindow(handle, SW_HIDE);

        ShowCredits();
        StartRockstarLauncher();
        StartService();
    }


    static void StartRockstarLauncher()
    {
        if(!File.Exists($@".\{filePlayGTAV}.exe"))
        {
            Console.WriteLine("[ERROR] {0} not found!", $@".\{filePlayGTAV}.exe");
            Environment.Exit(0);
        }
        else
        {
            Console.WriteLine("[DEBUG] Running Rockstar Launcher...");
            Process.Start($@".\{filePlayGTAV}.exe", commandLines);
        }
    }

    // Initializing check service...
    static void StartService() => new Thread(new ThreadStart(CheckRockstarService)).Start();

    static void CheckRockstarService()
    {
        while (true)
        {
            Thread.Sleep(1000);

            if (byPass && !isUpdating && IsRunning)
            {
                var process = Process.GetProcesses();
                var launcher = process.Where(item => item.ProcessName == "RockstarService" || item.ProcessName == "Launcher");
                if (process.Where(item => item.ProcessName == fileGTAVEnhanced || item.ProcessName == fileGTAV).Count() > 0)
                {
                    if (!CheckClose)
                    {
                        foreach (var app in launcher)
                            ShowWindow(app.Handle, SW_HIDE);
                        Console.WriteLine("[DEBUG] Checking game running.");
                    }
                    CheckClose = true;
                }
                else
                {
                    foreach (var app in launcher)
                    {
                        if (CheckClose)
                        {
                            // Close Rockstar Launcher and Process background
                            Console.WriteLine("[DEBUG] Exiting...");
                            process.Where(item => item.ProcessName == filePlayGTAV).First().Kill();
                            app.Kill();
                            Console.WriteLine(app.ProcessName);
                        }
                    }
                    if (launcher.Count() == 0)
                        break;
                }
                continue;
            }

            if (File.Exists(fileServiceLog))
            {
                // Load all lines in array
                var Lines = File.ReadAllLines(fileServiceLog);

                // Value initial to LastLines
                if (lastLine == 0)
                    lastLine = Lines.Length;

                // Load logs
                string logs = "";
                if (Lines.Length > lastLine)
                    foreach (var line in Lines.Skip(lastLine))
                        logs += line;

                // Check service stopped
                if (logs.Contains("Setting state to SERVICE_STOP"))
                {
                    // Update finish
                    if (isUpdating)
                    {
                        isUpdating = false;
                        byPass = false;
                    }
                    else
                    {
                        Console.WriteLine("[ERROR] Rockstar Service not initialized!");
                        break;
                    }
                }

                // Check Rockstar Launcher is updating
                if (!isUpdating && logs.Contains(@"Rockstar Games\Launcher\index.bin"))
                {
                    Console.WriteLine("[DEBUG] Rockstar Launcher is updating...");
                    isUpdating = true;
                }

                // Start Game
                if (byPass && !isUpdating)
                {
                    var file = File.Exists($@".\{fileGTAV}.exe") ? $@".\{fileGTAV}.exe" : File.Exists($@".\{fileGTAVEnhanced}.exe") ? $@".\{fileGTAVEnhanced}.exe" : null;

                    if (file == null)
                        Console.WriteLine("[ERROR] {0} or {1} not found!", $@".\{fileGTAV}.exe", $@".\{fileGTAVEnhanced}.exe");
                    else
                    {
                        Console.WriteLine("[DEBUG] Running GTA V...");
                        Process.Start(file, "-useEpic -fromRGL -EpicPortal");
                        IsRunning = true;
                    }
                    continue;
                }

                // Check service running
                if (logs.Contains("Setting state to SERVICE_RUNNING"))
                {
                    if (!byPass && !isUpdating)
                    {
                        byPass = true;
                        Console.WriteLine("[DEBUG] Running Rockstar Service...");
                    }
                }

                // Update LastLine
                lastLine = Lines.Length;
            }
        }
    }

    static void ShowCredits()
    {
        Console.WriteLine("================= Play GTA V Linux =================");
        Console.WriteLine("Store:   \t \t {0}", commandLines.Contains("-epicAppId=gta5") ? "Epic Games" : commandLines.Contains("-steamAppId=gta5") ? "Steam" : "Unknown");
        Console.WriteLine("Battle Eye: \t \t {0}", commandLines.Contains("-nobattleye") ? "Disabled" : "Enabled");
        Console.WriteLine("Github: \t \t https://github.com/treviasxk");
        Console.WriteLine("Repository: \t \t https://github.com/treviasxk/PlayGTAVLinux");
        Console.WriteLine("Version: \t \t 1.0.3.0");
        Console.WriteLine("Created by: \t \t Trevias Xk");
        Console.WriteLine("====================================================");
        Console.WriteLine("");
    }
}