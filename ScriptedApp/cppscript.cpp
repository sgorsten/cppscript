#include "cppscript.h"

#include <cassert>
#include <sstream>
#include <fstream>
#include <Windows.h>

namespace script
{
    static void RunSystemCommand(std::ostream & log, std::string cmd)
    {
        FILE * pipe = _popen(cmd.c_str(), "r");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), pipe)) log << buffer;
        fclose(pipe);
    }

    Library::Library(std::string name, std::string preamble) : name(move(name)), preamble(move(preamble)), module(), nextId() {}
    Library::~Library() { Unload(); }

    std::shared_ptr<_Node> Library::CreateScriptNode(const std::type_info & sig, std::string source)
    {
        // If signature and source match an existing function, hand that one out
        for (auto & n : nodes)
        {
            if (auto node = n.lock())
            {
                if (node->sig == sig.name() && node->source == source) return node;
            }
        }

        // Otherwise create a brand new function
        std::ostringstream ss;
        ss << "__script_function_" << nextId++;
        auto func = std::make_shared<_Node>();
        func->sig = sig.name();
        func->id = ss.str();
        func->source = source;
        func->impl = nullptr;
        nodes.push_back(func);
        return func;
    }

    void Library::Unload()
    {
        if (module)
        {
            std::vector<std::weak_ptr<_Node>> liveNodes;
            for (auto & n : nodes)
            {
                if (auto node = n.lock())
                {
                    node->impl = nullptr;
                    liveNodes.push_back(node);
                }
            }
            nodes = move(liveNodes);
            FreeLibrary((HMODULE)module);
            module = nullptr;
        }
    }

    void Library::Recompile(std::ostream & log)
    {
        Unload(); // Unload the current *.dll if one is loaded
        RunSystemCommand(log, "..\\scriptutils\\clean.bat " + name); // Clean existing artifacts and intermediates

        // Write script source code
        std::ofstream out("scripts\\" + name + "\\script.cpp");
        out << preamble;
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

    #ifdef NDEBUG
        const char * config = " RELEASE";
    #else
        const char * config = " DEBUG";
    #endif
        RunSystemCommand(log, "..\\scriptutils\\compile.bat " + name + config + (sizeof(void*) == 8 ? " x64" : " x86")); // Compile the new scripts

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
}