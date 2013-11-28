#include <string>
#include <iostream>
#include <fstream>
#include <Windows.h>

class ScriptNodeBase
{
    friend class ScriptEngine;
    std::string id, source;
    virtual const char * GetSignature() const = 0;
    virtual void Unload() = 0;
    virtual void Reload(void * loaderProc) = 0;
public:
    ScriptNodeBase(std::string id, std::string source) : id(move(id)), source(move(source)) {}
};

template<class Function> class ScriptNode : public ScriptNodeBase
{
    friend class ScriptEngine;
    Function * impl;
    ScriptNode(std::string id, std::string source) : ScriptNodeBase(move(id), move(source)), impl() {}
    const char * GetSignature() const { return typeid(Function).name(); }
    void Unload() { impl = nullptr; }
    void Reload(void * loaderProc) 
    { 
        typedef Function * FunctionPtr;
        auto loader = (FunctionPtr(*)())loaderProc;
        impl = loader();
    }
public:
    bool IsCompiled() const { return impl != nullptr; }
    Function * GetImpl() const { return impl; }
};

#include <vector>
#include <memory>
#include <map>
#include <sstream>
#include <cassert>
#include <typeinfo>
class ScriptEngine
{
    std::string name;
    std::map<const char *, std::pair<std::string, std::string>> sigs;
    std::vector<std::unique_ptr<ScriptNodeBase>> nodes;
    HMODULE module;
    size_t nextId;

    void RunSystemCommand(std::string cmd)
    {
        FILE * pipe = _popen(cmd.c_str(), "r");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), pipe)) std::cout << buffer << std::endl;
        fclose(pipe);
    }
public:
    ScriptEngine(std::string name) : name(move(name)), module(), nextId() {}
    ~ScriptEngine() { Unload(); }

    template<class Function> void DefineSignature(std::string returnType, std::string paramTypes)
    {
        sigs[typeid(Function).name()] = make_pair(move(returnType), move(paramTypes));
    }

    template<class Function> ScriptNode<Function> * CreateScript(std::string source) 
    { 
        std::ostringstream ss; 
        ss << "__script_function_" << nextId++;
        std::unique_ptr<ScriptNode<Function>> func(new ScriptNode<Function>(ss.str(), source));
        auto node = func.get(); 
        nodes.push_back(move(func));
        return node; 
    }

    void Unload()
    {
        if (module)
        {
            for (auto & n : nodes) n->Unload();
            FreeLibrary(module);
            module = nullptr;
        }
    }

    void Recompile()
    {
        Unload(); // Unload the current *.dll if one is loaded
        RunSystemCommand("clean.bat " + name); // Clean existing artifacts and intermediates

        std::string filename = "scripts\\" + name + "\\script.cpp";
        // Write script source code
        std::ofstream out(filename.c_str());
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            auto it = sigs.find(nodes[i]->GetSignature());
            assert(it != sigs.end()); // Must have defined signature ahead of time
            out << "extern \"C\" __declspec(dllexport) " << it->second.first << "(*" << nodes[i]->id << "())(" << it->second.second << ") { return []" << nodes[i]->source << "; }" << std::endl;
        }
        out.close();

        RunSystemCommand("compile.bat " + name); // Compile the new scripts

        // Load DLL and implementation of functions
        filename = "scripts\\" + name + "\\script.dll";
        module = LoadLibraryA(filename.c_str());
        if (!module) return; // Compilation failed
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            nodes[i]->Reload(GetProcAddress(module, nodes[i]->id.c_str()));
        }
    }
};

int main()
{
    ScriptEngine engine("test");

    engine.DefineSignature<int(int)>("int", "int");
    engine.DefineSignature<int(int,int)>("int", "int,int");

    auto sqr = engine.CreateScript<int(int)>("(int x) { return x*x; }");
    auto sum = engine.CreateScript<int(int,int)>("(int a, int b) { return a+b; }");

    engine.Recompile();
    std::cout << "sqr(5) = " << sqr->GetImpl()(5) << std::endl;
    std::cout << "sum(3,4) = " << sum->GetImpl()(3, 4) << std::endl;

    sqr = engine.CreateScript<int(int)>("(int x) { return x-2; }");

    engine.Recompile();
    std::cout << "sqr(5) = " << sqr->GetImpl()(5) << std::endl;
    std::cout << "sum(3,4) = " << sum->GetImpl()(3, 4) << std::endl;

    return 0;
}