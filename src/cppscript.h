#ifndef CPP_SCRIPT_H
#define CPP_SCRIPT_H

#include <utility>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace script
{
    struct _Node { const char * sig; std::string id, source; void * impl; };

    template<class Signature> class Function
    {
        std::shared_ptr<_Node>          node;
    public:
                                        Function() {}
                                        Function(std::shared_ptr<_Node> node) : node(move(node)) {}

        bool                            operator == (const Function & rhs) const { return node == rhs.node; }
        bool                            operator != (const Function & rhs) const { return node != rhs.node; }

        bool                            IsLoaded() const { return GetPointer() != nullptr; }
        const std::string &             GetSource() const { static const std::string empty; return node ? node->source : empty; }
        Signature *                     GetPointer() const { return node ? reinterpret_cast<Signature*>(node->impl) : nullptr; }

        template<class... Params> auto  operator()(Params... args) -> decltype(this->GetPointer()(args...)) const { return GetPointer()(args...); }
    };

    class Library
    {
        std::string name, preamble;
        std::map<const char *, std::pair<std::string, std::string>> sigs;
        std::vector<std::weak_ptr<_Node>> nodes;
        void * module;
        size_t nextId;

        std::shared_ptr<_Node> CreateScriptNode(const std::type_info & sig, std::string source);
    public:
        Library(std::string name, std::string preamble);
        ~Library();

        void Load();
        void Unload();
        void Recompile(std::ostream & log);

        template<class Signature> void DefineSignature(std::string returnType, std::string paramTypes) { sigs[typeid(Signature).name()] = make_pair(move(returnType), move(paramTypes)); }
        template<class Signature> Function<Signature> CreateScript(std::string source) { return CreateScriptNode(typeid(Signature), source); }
    };
}

#endif
