using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;

internal class Program{

    // Locations files
    static string filePlayGTAV = @".\PlayGTAV.exe";
    static string fileGTAV = @".\GTA5.exe";
    static string fileVersion = @".\version.txt";
    static string fileServiceLog = "C:/ProgramData/Rockstar Games/Launcher/service_log.txt";

    // CommandLines addicionals
    static string commandLines = "";
    static int lastLine = 0;

    static bool byPass = false, isUpdating = false;

    private static void Main(string[] args){
        foreach(var line in Environment.GetCommandLineArgs().Skip(1))
            commandLines += " " + line;
        ShowCredits();
        StartRockstarLauncher();
        StartService();
    }

    static void StartRockstarLauncher(){
        if(!File.Exists(filePlayGTAV)){
            Console.WriteLine("[ERROR] {0} not found!", filePlayGTAV);
            Environment.Exit(0);
        }else{
            Console.WriteLine("[DEBUG] Running Rockstar Launcher...");
            Process.Start(filePlayGTAV, commandLines);
        }
    }

    // Initializing check service...
    static void StartService() => new Thread(new ThreadStart(CheckRockstarService)).Start();

    static void CheckRockstarService(){
        while(true){
            Thread.Sleep(1000);
            if(File.Exists(fileServiceLog)){
                // Load all lines in array
                var Lines = File.ReadAllLines(fileServiceLog);

                // Value initial to LastLines
                if(lastLine == 0)
                    lastLine = Lines.Length;

                // Load logs
                string logs = "";
                if(Lines.Length > lastLine)
                    foreach(var line in Lines.Skip(lastLine))
                        logs += line;

                // Check service stopped
                if(logs.Contains("Setting state to SERVICE_STOP")){
                    // Update finish
                    if(isUpdating){
                        isUpdating = false; 
                        byPass = false;
                    }else{
                        Console.WriteLine("[ERROR] Rockstar Service not initialized!");
                        break;
                    }
                }

                // Check Rockstar Launcher is updating
                if(!isUpdating && logs.Contains(@"Rockstar Games\Launcher\index.bin")){
                    Console.WriteLine("[DEBUG] Rockstar Launcher is updating...");
                    isUpdating = true;
                }

                // Start Game
                if(byPass && !isUpdating){
                    if(!File.Exists(fileGTAV))
                        Console.WriteLine("[ERROR] {0} not found!", fileGTAV);
                    else{
                        Console.WriteLine("[DEBUG] Running GTA V...");
                        Process.Start(fileGTAV, "-useEpic -fromRGL -EpicPortal");

                        if(Process.GetProcessesByName("Launcher") is Process[] process){
                            // Minimize Rockstar Launcher                                    
                        }
                    }
                    break;
                }

                // Check service running
                if(logs.Contains("Setting state to SERVICE_RUNNING")){
                    if(!byPass && !isUpdating){
                        byPass = true;
                        Console.WriteLine("[DEBUG] Running Rockstar Service...");
                    }
                }
                
                // Update LastLine
                lastLine = Lines.Length;
            }
        }
    }

    static void ShowCredits(){
        Console.WriteLine("================= Play GTA V Linux =================");
        Console.WriteLine("Store:   \t \t {0}", commandLines.Contains("-epicAppId=gta5") ? "Epic Games" : commandLines.Contains("-steamAppId=gta5") ? "Steam" : "Unknown");
        Console.WriteLine("Battle Eye: \t \t {0}", commandLines.Contains("-nobattleye") ? "Disabled" : "Enabled");
        Console.WriteLine("Github: \t \t https://github.com/treviasxk");
        Console.WriteLine("Repository: \t \t https://github.com/treviasxk/PlayGTAVLinux");
        if(File.Exists(fileVersion))
            Console.WriteLine("GTA Version: \t \t {0}", File.ReadAllText(fileVersion));
        Console.WriteLine("Version: \t \t 1.0.0.0");
        Console.WriteLine("Created by: \t \t Trevias Xk");
        Console.WriteLine("====================================================");
        Console.WriteLine("");
    }
}