// Copyright (c) 2012-2020 Flax Engine. All rights reserved.

using System;
using System.Collections.Generic;
using Janus.Build.Projects.VisualStudioCode;

namespace Janus.Build.Projects {
    public abstract class ProjectGenerator
    {
        public abstract string ProjectFileExtension { get; }

        public abstract string SolutionFileExtension { get; }

        public abstract TargetType? Type { get; }

        public virtual Project CreateProject()
        {
            return new Project
            {
                Generator = this,
            };
        }
        public virtual Solution CreateSolution()
        {
            return new Solution();
        }
        public abstract void GenerateProject(Project project);

        public abstract void GenerateSolution(Solution solution);

        public static ProjectGenerator Create(ProjectFormat format, TargetType type)
        {
            switch (format)
            {
            case ProjectFormat.VisualStudioCode: return new VisualStudioCodeProjectGenerator();
            default: throw new ArgumentOutOfRangeException(nameof(format), "Unknown project format.");
            }
        }
    }
}
