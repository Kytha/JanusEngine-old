// Copyright (c) 2012-2020 Flax Engine. All rights reserved.

using System;
using System.IO;
using Janus.Build.Platforms;

namespace Janus.Build.Projects.VisualStudioCode
{
    public sealed class VisualStudioCodeInstance
    {
        private static VisualStudioCodeInstance _instance;
        public string Path;
        public static bool HasIDE()
        {
            return GetInstance() != null;
        }
        public static VisualStudioCodeInstance GetInstance()
        {
            if (_instance == null)
            {
                switch (Platform.NativePlatform.Target)
                {
                case TargetPlatform.Windows:
                {
                    if (!WindowsPlatformBase.TryReadDirRegistryKey("HKEY_CURRENT_USER\\SOFTWARE\\Classes\\Applications\\Code.exe\\shell\\open\\command", string.Empty, out var cmd))
                    {
                        if (!WindowsPlatformBase.TryReadDirRegistryKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Applications\\Code.exe\\shell\\open\\command", string.Empty, out cmd))
                        {
                            return null;
                        }
                    }
                    var path = cmd.Substring(1, cmd.Length - "\" \"%1\"".Length - 1);
                    if (File.Exists(path))
                    {
                        _instance = new VisualStudioCodeInstance
                        {
                            Path = path,
                        };
                    }
                    break;
                }
                case TargetPlatform.Linux:
                {
                    var path = "/usr/bin/code";
                    if (File.Exists(path))
                    {
                        _instance = new VisualStudioCodeInstance
                        {
                            Path = path,
                        };
                    }
                    break;
                }
                case TargetPlatform.Mac:
                {
                    var userFolder = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
                    var paths = new string[]
                    {
                        "/Applications/Visual Studio Code.app",
                        userFolder + "/Visual Studio Code.app",
                        userFolder + "/Downloads/Visual Studio Code.app",
                    };
                    foreach (var path in paths)
                    {
                        if (Directory.Exists(path))
                        {
                            _instance = new VisualStudioCodeInstance
                            {
                                Path = path,
                            };
                            break;
                        }
                    }
                    break;
                }
                }

                if (_instance != null)
                    Log.Verbose($"Found VS Code at {_instance.Path}");
            }

            return _instance;
        }
    }
}
