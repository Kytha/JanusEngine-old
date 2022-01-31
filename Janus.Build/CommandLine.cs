using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Reflection;

namespace Janus.Build {
    public class CommandLine
    {        
        public class Option
        {        
            public string Name;
            public string Value;

            public override string ToString()
            {
                return Name;
            }
        }

        public static string Get()
        {
            string[] args = Environment.GetCommandLineArgs();
            if(args.Length > 1)
            {
                string commandLine = Environment.CommandLine.Remove(0, args[0].Length + 2);
                commandLine = commandLine.Trim(' ');
                return commandLine;
            }
            return string.Empty;
        }
        public static Option[] GetOptions()
        {
            return GetOptions(Get());
        }
        public static Option[] GetOptions(string commandLine)
        {
            int hash = commandLine.GetHashCode();
            Option[] options;
            lock (_cacheLock)
            {
                if (hash != _cacheHash)
                {
                    _cacheHash = hash;
                    _cacheOptions = Parse(commandLine);
                }

                options = _cacheOptions;
            }

            return options;
        }

        public static Option[] Parse(string commandLine)
        {
            var options = new List<Option>();
            var length = commandLine.Length;

            for (int i = 0; i < length;)
            {
                // Skip white space
                while (i < length && char.IsWhiteSpace(commandLine[i]))
                    i++;

                // Read option prefix
                if (i == length)
                    break;
                var wholeQuote = commandLine[i] == '\"';
                if (wholeQuote)
                    i++;
                if (commandLine[i] == '-')
                    i++;
                else if (commandLine[i] == '/')
                    i++;

                // Skip white space
                while (i < length && char.IsWhiteSpace(commandLine[i]))
                    i++;

                // Read option name
                int nameStart = i;
                while (i < length && commandLine[i] != '-' && commandLine[i] != '=' && !char.IsWhiteSpace(commandLine[i]))
                    i++;
                int nameEnd = i;
                string name = commandLine.Substring(nameStart, nameEnd - nameStart);

                // Skip white space
                while (i < length && char.IsWhiteSpace(commandLine[i]))
                    i++;

                // Check if has no value
                if (i >= length - 1 || commandLine[i] != '=')
                {
                    options.Add(new Option
                    {
                        Name = name,
                        Value = string.Empty
                    });
                    if (wholeQuote)
                        i++;
                    if (i != length && commandLine[i] != '\"')
                        i++;
                    continue;
                }

                // Read value
                i++;
                int valueStart, valueEnd;
                if (commandLine[i] == '\"')
                {
                    valueStart = i + 1;
                    i++;
                    while (i < length && commandLine[i] != '\"')
                        i++;
                    valueEnd = i;
                    i++;
                }
                else if (commandLine[i] == '\'')
                {
                    valueStart = i + 1;
                    i++;
                    while (i < length && commandLine[i] != '\'')
                        i++;
                    valueEnd = i;
                    i++;
                }
                else if (wholeQuote)
                {
                    valueStart = i;
                    while (i < length && commandLine[i] != '\"')
                        i++;
                    valueEnd = i;
                    i++;
                }
                else
                {
                    valueStart = i;
                    while (i < length && commandLine[i] != ' ')
                        i++;
                    valueEnd = i;
                }
                string value = commandLine.Substring(valueStart, valueEnd - valueStart);
                options.Add(new Option
                {
                    Name = name,
                    Value = value.Trim()
                });
            }

            return options.ToArray();
        }

        public static string GetHelp(Type type)
        {
            StringWriter result = new StringWriter();
            result.WriteLine("Usage: Janus.Build.exe [options]");
            result.WriteLine("Options");

            var options = GetMembers(type);
            foreach (var option in options)
            {
                result.Write(" -"+ option.Key.Name);

                if (!string.IsNullOrEmpty(option.Key.ValueHint))
                {
                    result.Write("={0}", option.Key.ValueHint);
                }

                result.WriteLine();

                if (!string.IsNullOrEmpty(option.Key.Description))
                {
                    result.WriteLine("\t{0}", option.Key.Description.Replace(Environment.NewLine, Environment.NewLine + "\t"));
                }
                result.WriteLine();
            }
            return result.ToString();
        }

        public static Dictionary<CommandLineAttribute, MemberInfo> GetMembers(Type type)
        {
            if (type == null)
                throw new ArgumentNullException();

            var result = new Dictionary<CommandLineAttribute, MemberInfo>();

            var members = type.GetMembers(BindingFlags.Static | BindingFlags.Public);

            for (int i = 0; i < members.Length; i++)
            {
                var member = members[i];

                var attribute = member.GetCustomAttribute<CommandLineAttribute>();
                if (attribute != null)
                {
                    result.Add(attribute, member);
                }
            }

            return result;
        }
        public static void Configure(Type type)
        {
            Configure(type, Get());
        }
        public static void Configure(Type type, string commandLine)
        {
            Configure(GetMembers(type), null, commandLine);
        }

        private static void Configure(Dictionary<CommandLineAttribute, MemberInfo> members, object instance, string commandLine)
        {
            if (commandLine == null)
                throw new ArgumentNullException();

            // Process command line
            var options = GetOptions(commandLine);

            foreach (var e in members)
            {
                // Get option from command line
                var member = e.Value;
                var option = options.FirstOrDefault(x => string.Equals(x.Name, e.Key.Name, StringComparison.OrdinalIgnoreCase));
                if (option == null) {
                    continue;
                }

                // Convert text value to actual value object
                object value;
                Type type;
                
                if (member.MemberType == MemberTypes.Field) {
                    var field = (FieldInfo)member;
                    type = field.FieldType;
                }
                else if (member.MemberType == MemberTypes.Property) {
                    var property = (PropertyInfo)member;
                    type = property.PropertyType;
                }
                else {
                    throw new Exception("Unknown member type.");
                }
                try
                {
                    // Implicit casting to boolean value
                    if (type == typeof(bool) && string.IsNullOrEmpty(option.Value))
                    {
                        value = true;
                    }

                     else if (type.IsArray)
                     {
                        var elementType = type.GetElementType();
                        var values = option.Value.Split(',');
                        value = Array.CreateInstance(elementType, values.Length);
                        TypeConverter typeConverter = TypeDescriptor.GetConverter(elementType);
                        for (int i = 0; i < values.Length; i++)
                        {
                            ((Array)value).SetValue(typeConverter.ConvertFromString(values[i]), i);
                        }
                     }

                    else
                    {
                        TypeConverter typeConverter = TypeDescriptor.GetConverter(type);
                        value = typeConverter.ConvertFromString(option.Value);
                    }

                }
                catch (Exception ex)
                {
                    throw new Exception(string.Format("Failed to convert configuration property {0} value \"{1}\" to type {2}", member.Name, option.Value, type), ex);
                }

                try
                {
                    if (member.MemberType == MemberTypes.Field) {
                        var field = (FieldInfo)member;
                        field.SetValue(instance, value);
                    }
                    else if (member.MemberType == MemberTypes.Property) {
                        var property = (PropertyInfo)member;
                        property.SetValue(instance, value);
                    }
                }
                catch (Exception ex)
                {
                    throw new Exception(string.Format("Failed to set configuration property {0} with argument {1} to value \"{2}\"", member.Name, option.Name, option.Value), ex);
                }
            }
        }

        public static bool HasOption(string name)
        {
            return HasOption(name, Get());
        }
        private static bool HasOption(string name, string commandLine)
        {
            return commandLine.Length > 0 && GetOptions(commandLine).Any(p => (string.Equals(p.Name, name, StringComparison.OrdinalIgnoreCase)));
        }
        

        private static int _cacheHash;
        private static readonly object _cacheLock = new object();
        private static Option[] _cacheOptions;
    }
}