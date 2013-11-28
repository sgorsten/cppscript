#include "cppscript.h"

#include <cassert>
#include <sstream>
#include <fstream>

static void RunSystemCommand(std::ostream & log, std::string cmd)
{
    FILE * pipe = _popen(cmd.c_str(), "r");
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe)) log << buffer << std::endl;
    fclose(pipe);
}

ScriptEngine::ScriptEngine(std::string name) : name(move(name)), module(), nextId() {}
ScriptEngine::~ScriptEngine() { Unload(); }

std::shared_ptr<ScriptNode> ScriptEngine::CreateScriptNode(const std::type_info & sig, std::string source)
{
    std::ostringstream ss;
    ss << "__script_function_" << nextId++;
    auto func = std::make_shared<ScriptNode>();
    func->sig = sig.name();
    func->id = ss.str();
    func->source = source;
    func->impl = nullptr;
    nodes.push_back(func);
    return func;
}

void ScriptEngine::Unload()
{
    if (module)
    {
        for (auto & n : nodes)
        {
            if (auto node = n.lock())
            {
                node->impl = nullptr;
            }
        }
        FreeLibrary((HMODULE)module);
        module = nullptr;
    }
}

void ScriptEngine::Recompile(std::ostream & log)
{
    Unload(); // Unload the current *.dll if one is loaded
    RunSystemCommand(log, "clean.bat " + name); // Clean existing artifacts and intermediates

    // Write script source code
    std::ofstream out("scripts\\" + name + "\\script.cpp");
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        if (auto node = nodes[i].lock())
        {
            auto it = sigs.find(node->sig);
            assert(it != sigs.end()); // Must have defined signature ahead of time
            out << "extern \"C\" __declspec(dllexport) " << it->second.first << "(*" << node->id << "())(" << it->second.second << ") { return []" << node->source << "; }" << std::endl;
        }
    }
    out.close();

    RunSystemCommand(log, "compile.bat " + name); // Compile the new scripts

    // Load the newly compiled *.dll
    std::string libpath = "scripts\\" + name + "\\script.dll";
    module = LoadLibraryA(libpath.c_str());
    if (!module) return; // Compilation failed (TODO: Throw?)
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        if (auto node = nodes[i].lock())
        {
            auto loader = (void *(*)())GetProcAddress((HMODULE)module, node->id.c_str());
            node->impl = loader();
        }
    }
}