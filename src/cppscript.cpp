#include "cppscript.h"

#include <cassert>
#include <sstream>
#include <fstream>

struct __module { std::map<size_t, void**> funcs; };

namespace script
{
    static const char * GetExportSpecifier();
    static void CleanLibrary(std::ostream & log, const std::string & name);
    static void CompileLibrary(std::ostream & log, const std::string & name);
    static void * OpenLibrary(const std::string & name);
    static void * LoadSymbol(void * library, const std::string & id);
    static void CloseLibrary(void * library);

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
        func->hash = h;
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

        module = OpenLibrary(name);
        if (!module) throw std::runtime_error("Missing library: " + name);
        auto loader = (void(*)(__module &))LoadSymbol(module, "__load_functions");
        if (!loader) throw std::runtime_error("Missing entrypoint: __load_functions");
        std::vector<std::shared_ptr<_Node>> _nodes;
        __module mod;
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            if (auto node = nodes[i].lock())
            {
                _nodes.push_back(node);
                mod.funcs[node->hash] = &node->impl;
            }
        }
        loader(mod); // NOTE: This is currently not terribly safe, if the *.dll is out of date
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
            CloseLibrary(module);
            module = nullptr;
        }
    }

    void Library::Recompile(std::ostream & log)
    {
        Unload(); // Unload the current *.dll if one is loaded
        CleanLibrary(log, name); // Clean existing artifacts and intermediates

        // Write script source code
        std::ofstream out("scripts/" + name + "/script.cpp");
        out << preamble << R"(
#include <functional>
#include <map>
struct __module {
    std::map<size_t, void**> funcs;
};

extern "C" )" << GetExportSpecifier() << " void __load_functions(__module & __) {\n";
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            if (auto node = nodes[i].lock())
            {
                auto it = sigs.find(node->sig);
                assert(it != sigs.end()); // Must have defined signature ahead of time
                out << "    *(" << it->second.first << "(**)(" << it->second.second << "))__.funcs[" << node->hash << "] = []" << node->source << ";" << std::endl;
            }
        }
        out << "}" << std::endl;
        out.close();

        CompileLibrary(log, name); // Compile the new scripts
        Load(); // Load the newly compiled *.dll
    }
}

#ifdef WIN32

#include <Windows.h>
#ifdef NDEBUG
static const char * config = " RELEASE";
#else
static const char * config = " DEBUG";
#endif
static void RunSystemCommand(std::ostream & log, std::string cmd) { auto pipe = _popen(cmd.c_str(), "r"); char buffer[1024]; while (fgets(buffer, sizeof(buffer), pipe)) log << buffer; fclose(pipe); }
const char * script::GetExportSpecifier() { return "__declspec(dllexport)"; }
void script::CleanLibrary(std::ostream & log, const std::string & name) { RunSystemCommand(log, "..\\scriptutils\\clean.bat " + name); }
void script::CompileLibrary(std::ostream & log, const std::string & name){ RunSystemCommand(log, "..\\scriptutils\\compile.bat " + name + config + (sizeof(void*) == 8 ? " x64" : " x86")); }
void * script::OpenLibrary(const std::string & name) { auto path = "scripts\\" + name + ".dll"; return LoadLibraryA(path.c_str()); }
void * script::LoadSymbol(void * library, const std::string & id) { return GetProcAddress(reinterpret_cast<HMODULE>(library), id.c_str()); }
void script::CloseLibrary(void * library) { FreeLibrary(reinterpret_cast<HMODULE>(library)); }

#else

#include <cstdio>
#include <dlfcn.h>
static void RunSystemCommand(std::ostream & log, std::string cmd) { auto pipe = popen(cmd.c_str(), "r"); char buffer[1024]; while (fgets(buffer, sizeof(buffer), pipe)) log << buffer; fclose(pipe); }
const char * script::GetExportSpecifier() { return ""; }
void script::CleanLibrary(std::ostream & log, const std::string & name) 
{ 
    RunSystemCommand(log, "rm -rf scripts/"+name);
    RunSystemCommand(log, "rm -f scripts/lib"+name+".so");
    RunSystemCommand(log, "mkdir -p scripts/"+name);
}
void script::CompileLibrary(std::ostream & log, const std::string & name)
{ 
    RunSystemCommand(log, "g++ -std=c++11 scripts/"+name+"/script.cpp -shared -fPIC -o scripts/lib"+name+".so");
}
void * script::OpenLibrary(const std::string & name) { auto path = "scripts/lib" + name + ".so"; return dlopen(path.c_str(), RTLD_LOCAL | RTLD_LAZY); }
void * script::LoadSymbol(void * library, const std::string & id) { return dlsym(library, id.c_str()); }
void script::CloseLibrary(void * library) { dlclose(library); }

#endif
