#include <string>

template<class Function>
struct ScriptedFunction
{
    std::string source;
    Function * impl;
};

#include <iostream>
#include <fstream>
#include <Windows.h>

typedef int Function(int);

class ScriptNode
{
    friend class ScriptEngine;

    std::string source;
    Function * impl;

    ScriptNode(std::string source) : source(move(source)), impl() {}
public:
    bool IsCompiled() const { return impl; }
    int Call(int x) const { return impl(x); }
};

#include <vector>
#include <memory>
#include <sstream>
struct ScriptEngine
{
    typedef std::unique_ptr<ScriptNode> ScriptNodePtr;
    std::vector<ScriptNodePtr> nodes;
    HMODULE module;
public:
    ScriptEngine() : module() {}
    ~ScriptEngine() { Unload(); }

    ScriptNode * CreateScript(std::string source) { nodes.push_back(ScriptNodePtr(new ScriptNode(source))); return nodes.back().get(); }

    void Unload()
    {
        if (module)
        {
            for (auto & n : nodes) n->impl = nullptr;
            FreeLibrary(module);
            module = nullptr;
        }
    }

    void Recompile()
    {
        Unload();

        // Write script source code
        std::ofstream out("script.cpp");
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            out << "extern \"C\" __declspec(dllexport) int (*__get_script_function_" << i << "())(int) { return []" << nodes[i]->source << "; }" << std::endl;
        }
        out.close();

        // Compile the script
        FILE * pipe = _popen("compile.bat", "r");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), pipe))
        {
            std::cout << buffer << std::endl;
        }

        // Load DLL and implementation of functions
        module = LoadLibrary(L"..\\Debug\\dll.dll");
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            std::ostringstream ss; 
            ss << "__get_script_function_" << i;
            auto s = ss.str();
            auto loader = (int(*(*)())(int))GetProcAddress(module, s.c_str());
            nodes[i]->impl = loader();
        }
    }
};

int main()
{
    ScriptEngine engine;

    auto sqr = engine.CreateScript("(int x) { return x*x; }");
    auto cube = engine.CreateScript("(int x) { return x*x*x; }");

    engine.Recompile();
    std::cout << "sqr(5) = " << sqr->Call(5) << std::endl;
    std::cout << "cube(5) = " << cube->Call(5) << std::endl;

    sqr = engine.CreateScript("(int x) { return x-2; }");

    engine.Recompile();
    std::cout << "sqr(5) = " << sqr->Call(5) << std::endl;
    std::cout << "cube(5) = " << cube->Call(5) << std::endl;

    return 0;
}