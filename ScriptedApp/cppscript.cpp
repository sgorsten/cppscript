#include "cppscript.h"

static void RunSystemCommand(std::string cmd)
{
    FILE * pipe = _popen(cmd.c_str(), "r");
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe)) std::cout << buffer << std::endl;
    fclose(pipe);
}

ScriptEngine::ScriptEngine(std::string name) : name(move(name)), module(), nextId() {}
ScriptEngine::~ScriptEngine() { Unload(); }

std::string ScriptEngine::CreateScriptName()
{
    std::ostringstream ss;
    ss << "__script_function_" << nextId++;
    return ss.str();
}

void ScriptEngine::Unload()
{
    if (module)
    {
        for (auto & n : nodes)
        {
            if (auto node = n.lock())
            {
                node->Unload();
            }
        }
        FreeLibrary(module);
        module = nullptr;
    }
}

void ScriptEngine::Recompile()
{
    Unload(); // Unload the current *.dll if one is loaded
    RunSystemCommand("clean.bat " + name); // Clean existing artifacts and intermediates

    std::string filename = "scripts\\" + name + "\\script.cpp";
    // Write script source code
    std::ofstream out(filename.c_str());
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        if (auto node = nodes[i].lock())
        {
            auto it = sigs.find(node->GetSignature());
            assert(it != sigs.end()); // Must have defined signature ahead of time
            out << "extern \"C\" __declspec(dllexport) " << it->second.first << "(*" << node->id << "())(" << it->second.second << ") { return []" << node->source << "; }" << std::endl;
        }
    }
    out.close();

    RunSystemCommand("compile.bat " + name); // Compile the new scripts

    // Load DLL and implementation of functions
    filename = "scripts\\" + name + "\\script.dll";
    module = LoadLibraryA(filename.c_str());
    if (!module) return; // Compilation failed
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        if (auto node = nodes[i].lock())
        {
            node->Reload(GetProcAddress(module, node->id.c_str()));
        }
    }
}