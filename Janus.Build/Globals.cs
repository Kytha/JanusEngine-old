// Copyright (c) 2012-2022 Wojciech Figat. All rights reserved.

namespace Janus.Build
{
    public static partial class Globals
    {
        public static string Root = null;
        public static string EngineRoot = null;
        public static ProjectInfo Project;
        public static readonly TargetPlatform[] AllPlatforms =
        {
            TargetPlatform.Windows,
            TargetPlatform.XboxOne,
            TargetPlatform.Linux,
            TargetPlatform.PS4,
            TargetPlatform.PS5,
            TargetPlatform.Android,
            TargetPlatform.Switch,
            TargetPlatform.Mac,
        };
        public static readonly TargetArchitecture[] AllArchitectures =
        {
            TargetArchitecture.x64,
            TargetArchitecture.x86,
            TargetArchitecture.ARM,
            TargetArchitecture.ARM64,
        };
        public static readonly TargetConfiguration[] AllConfigurations =
        {
            TargetConfiguration.Debug,
            TargetConfiguration.Development,
            TargetConfiguration.Release,
        };
    }
}
