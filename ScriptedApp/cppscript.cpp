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
        // Compute a name for this function by hashing the signature and source code together
        std::ostringstream ss;
        ss << sig.name() << " " << source;
        auto h = std::hash<std::string>()(ss.str());
        ss.str("");
        ss << "__script_function_" << h;
        std::string id = ss.str();

        // If signature and source match an existing function, hand that one out
        for (auto & n : nodes)
        {
            if (auto node = n.lock())
            {
                if (node->sig == sig.name() && node->source == source) return node;
                if (node->id == id) throw std::runtime_error("Hash collision in library " + name + ": " + source); // User can always add whitespace to change the hash
            }
        }

        // Create a script node for this function
        auto func = std::make_shared<_Node>();
        func->sig = sig.name();
        func->id = ss.str();
        func->source = source;
        func->impl = nullptr;
        nodes.push_back(func);
        return func;
    }

    void Library::Load()
    {
        Unload();

        std::string libpath = "scripts\\" + name + ".dll";
        module = LoadLibraryA(libpath.c_str());
        if (!module) throw std::runtime_error("Missing library: " + name);
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            if (auto node = nodes[i].lock())
            {
                auto loader = (void *(*)())GetProcAddress((HMODULE)module, node->id.c_str());
                if (!loader) throw std::runtime_error("Missing procedure in library " + name + ": " + node->source);
                node->impl = loader();
            }
        }
    }

    void Library::Unload()
    {
        // Unload implementations of all functions (and garbage-collect unreferenced functions)
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

        // Unload the actual *.dll
        if (module)
        {
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
        Load(); // Load the newly compiled *.dll
    }
}