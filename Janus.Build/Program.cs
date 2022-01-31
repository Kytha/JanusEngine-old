using System;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Net;
using System.Threading;

namespace Janus.Build
{
    class Program
    {
        static int Main()
        {
            if (CommandLine.HasOption("help"))
            {
                Console.WriteLine(CommandLine.GetHelp(typeof(Configuration)));
                return 0;
            }


            Stopwatch stopwatch = Stopwatch.StartNew();


            try {
                CommandLine.Configure(typeof(Configuration));
                Log.Init();
                foreach (var option in CommandLine.GetOptions())
                {
                    if (option.Name.Length > 1 && option.Name[0] == 'D')
                    {
                        var define = option.Name.Substring(1);
                        if (!string.IsNullOrEmpty(option.Value))
                            define += "=" + option.Value;
                        Configuration.CustomDefines.Add(define);
                    }
                }

                if (Configuration.CurrentDirectory != null)
                    Environment.CurrentDirectory = Configuration.CurrentDirectory;
                Globals.Root = Directory.GetCurrentDirectory();
                var executingAssembly = System.Reflection.Assembly.GetExecutingAssembly();
                Globals.EngineRoot = Utilities.RemovePathRelativeParts(Path.Combine(Path.GetDirectoryName(executingAssembly.Location), ".."));

                Version version = executingAssembly.GetName().Version;
                string versionString = string.Join(".", version.Major, version.Minor, version.Build);
                Log.Info(string.Format("Flax.Build {0}", versionString));
                using (new LogIndentScope())
                {
                    Log.Verbose("Arguments: " + CommandLine.Get());
                    Log.Verbose("Workspace: " + Globals.Root);
                    Log.Verbose("Engine: " + Globals.EngineRoot);
                }

                {
                    var projectFiles = Directory.GetFiles(Globals.Root, "*.janusproj", SearchOption.TopDirectoryOnly);
                    if (projectFiles.Length == 1)
                        Globals.Project = ProjectInfo.Load(projectFiles[0]);
                    else if (projectFiles.Length > 1)
                        throw new Exception("Too many project files. Don't know which to pick.");
                    else
                        Log.Warning("Missing project file.");
                }


                if (Configuration.GenerateProject)
                {
                    Log.Info("Generating project files...");
                    Thread.Sleep(1000);
                    Builder.GenerateProjects();
                }
            } catch (Exception ex)
            {
                Log.Exception(ex);
                return 1;
            } 
            finally
            {
                stopwatch.Stop();
                Log.Info(string.Format("Total time: {0}", stopwatch.Elapsed));
                Log.Verbose("End.");
                Log.Dispose();
            }
            
            return 0;
        }
    }
}