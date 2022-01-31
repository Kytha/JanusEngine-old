using System.Collections.Generic;

namespace Janus.Build
{
    public static partial class Configuration
    {
        [CommandLine("workspace", "<path>", "The custom working directory.")]
        public static string CurrentDirectory = null;

        [CommandLine("genproject", "Generates the projects for the workspace.")]
        public static bool GenerateProject = true;

        [CommandLine("BuildDeps", "Runs the deps building tool to fetch and compiles the 3rd party files to produce binaries (build missing ones).")]
        public static bool BuildDeps = false;

        [CommandLine("ReBuildDeps", "Runs the deps building tool to fetch and compiles the 3rd party files to produce binaries (force rebuild).")]
        public static bool ReBuildDeps = false;

        [CommandLine("deploy", "Runs the deploy tool.")]
        public static bool Deploy = false;

        [CommandLine("build", "Builds the targets.")]
        public static bool Build = false;

        [CommandLine("clean", "Cleans the build system cache.")]
        public static bool Clean = false;

        [CommandLine("rebuild", "Rebuilds the targets.")]
        public static bool Rebuild = false;

        [CommandLine("printSDKs", "Prints all SDKs found on system. Can be used to query Win10 SDK or any other platform-specific toolsets used by build tool.")]
        public static bool PrintSDKs = false;

        [CommandLine("logfile", "<path>", "The log file path relative to the working directory. Set to empty to disable it/")]
        public static string LogFile = "Cache/Intermediate/Log.txt";

        [CommandLine("verbose", "Enables verbose logging and detailed diagnostics.")]
        public static bool Verbose = true;

        [CommandLine("log", "Enables logging into console.")]
        public static bool ConsoleLog = true;
        //[CommandLine("configuration", "Debug", "The target configuration to build. If not specified builds all supported configurations.")]
        //public static TargetConfiguration[] BuildConfigurations;

        //[CommandLine("platform", "Windows", "The target platform to build. If not specified builds all supported platforms.")]
        //public static TargetPlatform[] BuildPlatforms;

        //[CommandLine("arch", "<x64/x86/arm/arm64>", "The target platform architecture to build. If not specified builds all valid architectures.")]
        //public static TargetArchitecture[] BuildArchitectures;

        [CommandLine("maxConcurrency", "<threads>", "The maximum allowed concurrency for a build system (maximum active worker threads count).")]
        public static int MaxConcurrency = 1410;

        [CommandLine("concurrencyProcessorScale", "<scale>", "The concurrency scale for a build system that specifies how many worker threads allocate per-logical processor.")]
        public static float ConcurrencyProcessorScale = 1.0f;

        [CommandLine("binaries", "<path>", "The output binaries folder path relative to the working directory.")]
        public static string BinariesFolder = "Binaries";

        [CommandLine("intermediate", "<path>", "The intermediate build files folder path relative to the working directory.")]
        public static string IntermediateFolder = "Cache/Intermediate";

        [CommandLine("vs2015", "Generates Visual Studio 2015 project format files. Valid only with -genproject option.")]
        public static bool ProjectFormatVS2015 = false;

        [CommandLine("vs2017", "Generates Visual Studio 2017 project format files. Valid only with -genproject option.")]
        public static bool ProjectFormatVS2017 = false;

        [CommandLine("vs2019", "Generates Visual Studio 2019 project format files. Valid only with -genproject option.")]
        public static bool ProjectFormatVS2019 = false;

        [CommandLine("vs2022", "Generates Visual Studio 2022 project format files. Valid only with -genproject option.")]
        public static bool ProjectFormatVS2022 = false;

        [CommandLine("vscode", "Generates Visual Studio Code project format files. Valid only with -genproject option.")]
        public static bool ProjectFormatVSCode = false;

        /// <summary>
        /// Custom configuration defines provided via command line for the build tool.
        /// </summary>
        public static List<string> CustomDefines = new List<string>();
    }
}