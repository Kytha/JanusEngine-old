// Copyright (c) 2012-2022 Wojciech Figat. All rights reserved.

using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using Newtonsoft.Json;

namespace Janus.Build
{
    public sealed class ProjectInfo
    {
        private static List<ProjectInfo> _projectsCache;

        /// <summary>
        /// The project reference.
        /// </summary>
        public class Reference
        {
            public string Name;
            public ProjectInfo Project;

            public override string ToString()
            {
                return Name;
            }
        }

        public string Name;

        [NonSerialized]
        public string ProjectPath;

        [NonSerialized]
        public string ProjectFolderPath;
        public Version Version;
        public string Company = string.Empty;
        public string Copyright = string.Empty;
        public string GameTarget;
        public string EditorTarget;
        public Reference[] References = new Reference[0];
        public Version MinEngineVersion;
        public HashSet<ProjectInfo> GetAllProjects()
        {
            var result = new HashSet<ProjectInfo>();
            GetAllProjects(result);
            return result;
        }

        private void GetAllProjects(HashSet<ProjectInfo> result)
        {
            result.Add(this);
            foreach (var reference in References)
                reference.Project.GetAllProjects(result);
        }

        /// <summary>
        /// Saves the project file.
        /// </summary>
        public void Save()
        {
            var contents = JsonConvert.SerializeObject(this);
            File.WriteAllText(ProjectPath, contents);
        }

        /// <summary>
        /// Loads the project from the specified file.
        /// </summary>
        /// <param name="path">The path.</param>
        /// <returns>The loaded project.</returns>
        public static ProjectInfo Load(string path)
        {
            // Try to reuse loaded file
            path = Utilities.RemovePathRelativeParts(path);
            if (_projectsCache == null)
                _projectsCache = new List<ProjectInfo>();
            for (int i = 0; i < _projectsCache.Count; i++)
            {
                if (_projectsCache[i].ProjectPath == path)
                    return _projectsCache[i];
            }

            try
            {
                // Load
                Log.Verbose("Loading project file from \"" + path + "\"...");
                var contents = File.ReadAllText(path);
                var project = JsonConvert.DeserializeObject<ProjectInfo>(contents);
                project.ProjectPath = path;
                project.ProjectFolderPath = Path.GetDirectoryName(path);

                // Process project data
                if (string.IsNullOrEmpty(project.Name))
                    throw new Exception("Missing project name.");
                if (project.Version == null)
                    project.Version = new Version(1, 0);
                if (project.Version.Revision == 0)
                    project.Version = new Version(project.Version.Major, project.Version.Minor, project.Version.Build);
                if (project.Version.Build == 0 && project.Version.Revision == -1)
                    project.Version = new Version(project.Version.Major, project.Version.Minor);
                foreach (var reference in project.References)
                {
                    string referencePath;
                    if (reference.Name.StartsWith("$(EnginePath)"))
                    {
                        // Relative to engine root
                        referencePath = Path.Combine(Globals.EngineRoot, reference.Name.Substring(14));
                    }
                    else if (reference.Name.StartsWith("$(ProjectPath)"))
                    {
                        // Relative to project root
                        referencePath = Path.Combine(project.ProjectFolderPath, reference.Name.Substring(15));
                    }
                    else if (Path.IsPathRooted(reference.Name))
                    {
                        // Relative to workspace
                        referencePath = Path.Combine(Environment.CurrentDirectory, reference.Name);
                    }
                    else
                    {
                        // Absolute
                        referencePath = reference.Name;
                    }
                    referencePath = Utilities.RemovePathRelativeParts(referencePath);

                    // Load referenced project
                    reference.Project = Load(referencePath);
                }

                // Project loaded
                Log.Verbose(String.Format("Loaded project {0}, version {1}",project.Name,project.Version));
                _projectsCache.Add(project);
                return project;
            }
            catch
            {
                // Failed to load project
                Log.Error("Failed to load project \"" + path + "\".");
                throw;
            }
        }

        /// <inheritdoc />
        public override string ToString()
        {
            return Name;
        }

        /// <inheritdoc />
        public override bool Equals(object obj)
        {
            var info = obj as ProjectInfo;
            return info != null &&
                   Name == info.Name &&
                   ProjectPath == info.ProjectPath &&
                   ProjectFolderPath == info.ProjectFolderPath &&
                   EqualityComparer<Version>.Default.Equals(Version, info.Version) &&
                   Company == info.Company &&
                   Copyright == info.Copyright &&
                   EqualityComparer<Reference[]>.Default.Equals(References, info.References);
        }

        /// <inheritdoc />
        public override int GetHashCode()
        {
            var hashCode = -833167044;
            hashCode = hashCode * -1521134295 + EqualityComparer<string>.Default.GetHashCode(Name);
            hashCode = hashCode * -1521134295 + EqualityComparer<string>.Default.GetHashCode(ProjectPath);
            hashCode = hashCode * -1521134295 + EqualityComparer<string>.Default.GetHashCode(ProjectFolderPath);
            hashCode = hashCode * -1521134295 + EqualityComparer<Version>.Default.GetHashCode(Version);
            hashCode = hashCode * -1521134295 + EqualityComparer<string>.Default.GetHashCode(Company);
            hashCode = hashCode * -1521134295 + EqualityComparer<string>.Default.GetHashCode(Copyright);
            hashCode = hashCode * -1521134295 + EqualityComparer<Reference[]>.Default.GetHashCode(References);
            return hashCode;
        }
    }
}
