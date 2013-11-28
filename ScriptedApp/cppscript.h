#ifndef CPP_SCRIPT_H
#define CPP_SCRIPT_H

#include <string>
#include <iostream>
#include <fstream>
#include <Windows.h>

#include <vector>
#include <memory>
#include <map>
#include <sstream>
#include <cassert>
#include <typeinfo>

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
    const char * GetSignature() const { return typeid(Function).name(); }
    void Unload() { impl = nullptr; }
    void Reload(void * loaderProc)
    {
        typedef Function * FunctionPtr;
        auto loader = (FunctionPtr(*)())loaderProc;
        impl = loader();
    }
public:
    ScriptNode(std::string id, std::string source) : ScriptNodeBase(move(id), move(source)), impl() {}
    bool IsCompiled() const { return impl != nullptr; }
    Function * GetImpl() const { return impl; }
};

template<class Function> class ScriptFunction
{
    std::shared_ptr<ScriptNode<Function>> node;
public:
    ScriptFunction() {}
    ScriptFunction(std::shared_ptr<ScriptNode<Function>> node) : node(move(node)) {}

    bool IsCompiled() const { return node && node->IsCompiled(); }
    Function * GetImpl() const { return node ? node->GetImpl() : nullptr; }
    template<class... Params> auto operator()(Params... args) -> decltype(GetImpl()(args...)) const { return GetImpl()(args...); }
};

class ScriptEngine
{
    std::string name;
    std::map<const char *, std::pair<std::string, std::string>> sigs;
    std::vector<std::weak_ptr<ScriptNodeBase>> nodes;
    HMODULE module;
    size_t nextId;

    std::string CreateScriptName();
public:
    ScriptEngine(std::string name);
    ~ScriptEngine();

    void Unload();
    void Recompile();

    template<class Function> void DefineSignature(std::string returnType, std::string paramTypes)
    {
        sigs[typeid(Function).name()] = make_pair(move(returnType), move(paramTypes));
    }

    template<class Function> ScriptFunction<Function> CreateScript(std::string source)
    {
        auto func = std::make_shared<ScriptNode<Function>>(CreateScriptName(), source);
        nodes.push_back(func);
        return func;
    }
};

#endif