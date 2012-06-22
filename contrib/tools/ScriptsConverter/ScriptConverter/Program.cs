#region

using System;
using System.Collections;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

#endregion

namespace ScriptConverter
{
    internal class Program
    {
        private static void Main(string[] args)
        {
            if (args.Length != 1)
            {
                Console.WriteLine("Usage: ScriptsConverter.exe [path_to_dir|path_to_file]");
            }
            else
            {
                string path = args[0];
                if (File.Exists(path))
                {
                    ProcessFile(path);
                }
                else if (!Directory.Exists(path))
                {
                    Console.WriteLine(
                        "Invalid file or directory specified.\r\n\r\nUsage: ScriptsConverter.exe [path_to_dir|path_to_file]");
                }
                else
                {
                    ProcessDirectory(path);
                }
            }
        }

        private static void ProcessDirectory(string path)
        {
            String[] files = Directory.GetFiles(path, "*.cpp");
            foreach (String file in files)
            {
                ProcessFile(file);
            }
            String[] dirs = Directory.GetDirectories(path);
            foreach (String dir in dirs)
            {
                ProcessDirectory(dir);
            }
        }

        private static string GetMethod(string method, ref string txt, ref int minPos)
        {
            Match m = new Regex(method + "(\\s|:|[(])").Match(txt);
            var pos = m.Index;
            Int32 lastPos = txt.IndexOf("\n}", pos, StringComparison.Ordinal);
            if (m.Success)
            {
                while (pos-- >= 0 && pos < txt.Length)
                    if (txt[pos] == '\n') break;
                //pos++;
                if (lastPos != -1)
                {
                    lastPos += 2;
                    while (lastPos++ >= 0 && lastPos < txt.Length)
                    {
                        if (txt[lastPos] == '\n') break;
                    }
                    return txt.Substring(pos, lastPos - pos);
/*
                    txt = txt.Remove(pos, lastPos - pos);
*/
                }
                if (pos < minPos)
                    minPos = pos;
            }
            return null;
        }

        private static void ProcessFile(string filePath)
        {
            Console.WriteLine(filePath);

            string txt = File.ReadAllText(filePath);
            string[] lines = File.ReadAllLines(filePath);
            Array.Reverse(lines);

            ScriptData data = null;
            bool scriptStart = false;
            var scripts = new ArrayList();
            foreach (string line in lines)
            {
                if (line.IndexOf("Script *", StringComparison.Ordinal) != -1)
                {
                    break;
                }
                if (line.IndexOf("->RegisterSelf();", StringComparison.Ordinal) != -1)
                {
                    scriptStart = true;
                    data = new ScriptData();
                    continue;
                }
                if (scriptStart)
                {
                    if (line.IndexOf("= new Script", StringComparison.Ordinal) != -1)
                    {
                        scriptStart = false;
                        scripts.Add(data);
                        data = null;
                        continue;
                    }
                    var r = new Regex("newscript->([a-zA-Z]+) *= *&?([\"_a-zA-Z0-9]+);");
                    Match m = r.Match(line);
                    if (m.Success)
                    {
                        if (m.Groups[1].Value.Equals("Name"))
                        {
                            data.Name = m.Groups[2].Value.Trim(new[] {'"'});
                        }
                        else
                        {
                            data.AddMethod(m.Groups[2].Value);
                        }
                    }
                }
            }
            if (scripts.Count != 0)
            {
                string register = "";
                foreach (ScriptData sd in scripts)
                {
                    Console.WriteLine(sd);
                    int minPos = txt.Length;
                    String ss =
                        (from string method in sd.Methods select GetMethod(method, ref txt, ref minPos)).Aggregate("",
                                                                                                                   (
                                                                                                                       current,
                                                                                                                       s)
                                                                                                                   =>
                                                                                                                   current +
                                                                                                                   (s +
                                                                                                                    "\n"));
                    if (sd.InstanceName != null)
                    {
                        String s = GetMethod("struct " + sd.InstanceName, ref txt, ref minPos);
                        ss += s + "\n";
                    }
                    if (sd.AiName != null)
                    {
                        string ai = GetMethod("struct " + sd.AiName, ref txt, ref minPos);
                        if (ai != null)
                        {
                            var r = new Regex("\\S+ " + sd.AiName + "::([^( ]+)");
                            while (r.IsMatch(txt))
                            {
                                Match m = r.Match(txt);
                                int startPos = m.Index;
                                int endPos = txt.IndexOf("\n}", startPos, StringComparison.Ordinal);
                                if (endPos != -1)
                                    endPos += 2;
                                while (endPos++ >= 0 && endPos < txt.Length)
                                {
                                    if (txt[endPos] == '\n') break;
                                }
                                string sm = txt.Substring(startPos, endPos - startPos);
                                txt = txt.Remove(startPos, endPos - startPos);
                                {
                                    sm = sm.Replace("\n", "\n    ");
                                    var r1 = new Regex("\\S+ " + m.Groups[1] + " *\\([^)]*\\) *;");
                                    Match m1 = r1.Match(ai);
                                    if (m1.Success)
                                    {
                                        ai = r1.Replace(ai, sm);
                                    }
                                }
                            }
                            ai = ai.Replace(sd.AiName + "::", "");
                            ss += ai + "\n";
                        }
                    }
                    if (ss.Length != 0)
                    {
                        string typeName = "UnknownScript";
                        switch (sd.Type)
                        {
                            case 1:
                                typeName = "CreatureScript";
                                break;
                            case 2:
                                typeName = "InstanceMapScript";
                                break;
                            default:
                                if (sd.Name.IndexOf("npc", StringComparison.Ordinal) == 0)
                                    typeName = "CreatureScript";
                                else if (sd.Name.IndexOf("mob", StringComparison.Ordinal) == 0)
                                    typeName = "CreatureScript";
                                else if (sd.Name.IndexOf("boss_", StringComparison.Ordinal) == 0)
                                    typeName = "CreatureScript";
                                else if (sd.Name.IndexOf("item_", StringComparison.Ordinal) == 0)
                                    typeName = "ItemScript";
                                else if (sd.Name.IndexOf("go_", StringComparison.Ordinal) == 0)
                                    typeName = "GameObjectScript";
                                else if (sd.Name.IndexOf("at_", StringComparison.Ordinal) == 0)
                                    typeName = "AreaTriggerScript";
                                else if (sd.Name.IndexOf("instance_", StringComparison.Ordinal) == 0)
                                    typeName = "InstanceMapScript";
                                break;
                        }
                        if (sd.InstanceName != null)
                            ss = ss.Replace(sd.InstanceName, sd.InstanceName + "_InstanceMapScript");
                        ss = ss.Replace("\n", "\n    ");
                        ss = "class " + sd.Name + " : public " + typeName + "\n{\npublic:\n    " +
                             sd.Name + "() : " + typeName + "(\"" + sd.Name + "\") { }\n" + ss + "\n};";
                        ss = ss.Replace("_" + sd.Name, "");
                        ss = ss.Replace("AIAI", "AI");
                        ss = ss.Replace("    \r\n", "\r\n");
                        ss = ss.Replace("    \n", "\n");
                        txt = txt.Insert(minPos, ss);
                        register = "    new " + sd.Name + "();\n" + register;
                    }
                }
                var r2 = new Regex("void +AddSC_([_a-zA-Z0-9]+)");
                Match m2 = r2.Match(txt);
                if (m2.Success)
                {
                    txt = txt.Remove(m2.Index);
                    txt += "void AddSC_" + m2.Groups[1].Value + "()\n{\n" + register + "}\n";
                }
                // File.Copy(filePath, filePath + ".bkp");
                txt = txt.Replace("\r\n", "\n");
                File.WriteAllText(filePath, txt);
            }
        }

        #region Nested type: ScriptData

        private class ScriptData
        {
            public readonly ArrayList Methods = new ArrayList();
            private readonly string[] _special = new[] {"GetAI_", "GetInstance_", "GetInstanceData_"};
            public string AiName;
            public string InstanceName;
            public string Name;
            public int Type;

            public void AddMethod(string method)
            {
                Methods.Add(method);
                int i = 0;
                foreach (string s in _special)
                {
                    ++i;
                    int pos = method.IndexOf(s, StringComparison.Ordinal);
                    string name = method.Substring(pos + s.Length);
                    if (pos != -1)
                    {
                        Type = i;
                        if (i == 1)
                        {
                            AiName = name + "AI";
                        }
                        if (i == 2 || i == 3)
                            InstanceName = name;
                    }
                }
            }

            public override string ToString()
            {
                var sb = new StringBuilder();
                sb.AppendFormat("Script: {0}\n", Name);
                foreach (string method in Methods)
                    sb.Append("    ").Append(method).Append("\n");
                sb.Append("\n");
                return sb.ToString();
            }
        }

        #endregion
    }
}