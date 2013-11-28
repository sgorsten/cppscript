#include "cppscript.h"

#include <iostream>

int main()
{
    ScriptEngine engine("test", "#include <iostream>\n");

    engine.DefineSignature<void()>("void", "");
    engine.DefineSignature<int(int)>("int", "int");
    engine.DefineSignature<double(double,double)>("double", "double,double");

    auto hello = engine.CreateScript<void()>("() { std::cout << \"Hello world!\" << std::endl; }");
    engine.Recompile(std::cout);
    hello();

    hello = engine.CreateScript<void()>("() { std::cout << \"Scripting in C++ is surprisingly simple!\" << std::endl; }");
    auto sqr = engine.CreateScript<int(int)>("(int x) { return x*x; }");
    auto sum = engine.CreateScript<double(double,double)>("(double a, double b) { return a+b; }");
    engine.Recompile(std::cout);

    std::cout << "sqr(5) = " << sqr(5) << std::endl;
    std::cout << "sum(3.1,4.2) = " << sum(3.1, 4.2) << std::endl;
    hello();

    return 0;
}