#ifndef CPP_SCRIPT_H
#define CPP_SCRIPT_H

#include <functional>
#include <utility>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace script
{
    struct _Node { const char * sig; size_t hash; std::string source; std::shared_ptr<void> impl; };

    template<class Signature> class Function
    {
        std::shared_ptr<_Node>          node;
    public:
                                        Function() {}
                                        Function(std::shared_ptr<_Node> node) : node(move(node)) {}

        bool                            operator == (const Function & rhs) const { return node == rhs.node; }
        bool                            operator != (const Function & rhs) const { return node != rhs.node; }

        bool                            IsLoaded() const { return GetFunction(); }
        const std::string &             GetSource() const { static const std::string empty; return node ? node->source : empty; }
        std::function<Signature>        GetFunction() const { static const std::function<Signature> empty; return (node && node->impl) ? *reinterpret_cast<std::function<Signature> *>(node->impl.get()) : empty; }

#ifdef WIN32
        template<class... Params> auto  operator()(Params... args) -> decltype(GetFunction()(args...)) const { return GetFunction()(args...); }
#else
        template<class... Params> auto  operator()(Params... args) -> decltype(this->GetFunction()(args...)) const { return GetFunction()(args...); }
#endif
    };
    typedef Function<void()> Action;

    class Context
    {
        std::string name;
        std::map<const char *, std::string> sigs;
        std::vector<std::weak_ptr<_Node>> nodes;
        std::map<std::string, std::pair<std::string, void *>> vars;
        std::vector<std::shared_ptr<void>> ownedVars;
        void * module;

        std::vector<std::string> commonHeaders;
        std::vector<std::string> commonLibraries;

        std::shared_ptr<_Node> CreateScriptNode(const std::type_info & sig, std::string source);
    public:
        Context(std::string name);
        ~Context();

        void AddCommonHeader(const std::string & path) { commonHeaders.push_back(path); }
        void AddCommonLibrary(const std::string & path) { commonLibraries.push_back(path); }

        void Load();
        void Unload();
        void Recompile(std::ostream & log);

        void AddVariableReference(const char * name, std::string type, void * ptr) { vars[name] = std::make_pair(type,ptr); }
        template<class T> void CreateVariable(const char * name, std::string type, T val) { auto ptr = std::make_shared<T>(std::move(val)); AddVariableReference(name, type, ptr.get()); ownedVars.push_back(ptr); }

        template<class Signature> void DefineSignature(std::string sig) { sigs[typeid(Signature).name()] = sig; }
        template<class Signature> Function<Signature> CreateFunction(std::string source) { return CreateScriptNode(typeid(Signature), source); }
        template<class T> Function<T()> CreateExpression(std::string source) { return CreateFunction<T()>("() { return " + source + "; }"); }
        Action CreateAction(std::string source) { return CreateFunction<void()>("() { " + source + " }"); }
    };
}

#endif
