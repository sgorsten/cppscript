#ifndef CPP_SCRIPT_H
#define CPP_SCRIPT_H

#include <memory>
#include <string>
#include <vector>
#include <map>

#include <Windows.h>

struct ScriptNode
{
    const char * sig;
    std::string id, source;
    void * impl;
};

template<class Function> class ScriptFunction
{
    std::shared_ptr<ScriptNode> node;
public:
    ScriptFunction() {}
    ScriptFunction(std::shared_ptr<ScriptNode> node) : node(move(node)) {}

    bool IsCompiled() const { return node && node->impl; }
    Function * GetImpl() const { return node ? reinterpret_cast<Function*>(node->impl) : nullptr; }
    template<class... Params> auto operator()(Params... args) -> decltype(GetImpl()(args...)) const { return GetImpl()(args...); }
};

class ScriptEngine
{
    std::string name;
    std::map<const char *, std::pair<std::string, std::string>> sigs;
    std::vector<std::weak_ptr<ScriptNode>> nodes;
    void * module;
    size_t nextId;

    std::shared_ptr<ScriptNode> CreateScriptNode(const std::type_info & sig, std::string source);
public:
    ScriptEngine(std::string name);
    ~ScriptEngine();

    void Load();
    void Unload();
    void Recompile(std::ostream & log);

    template<class Function> void DefineSignature(std::string returnType, std::string paramTypes) { sigs[typeid(Function).name()] = make_pair(move(returnType), move(paramTypes)); }
    template<class Function> ScriptFunction<Function> CreateScript(std::string source) { return CreateScriptNode(typeid(Function), source); }
};

#endif