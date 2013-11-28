#include "cppscript.h"

int main()
{
    ScriptEngine engine("test");

    engine.DefineSignature<int(int)>("int", "int");
    engine.DefineSignature<int(int,int)>("int", "int,int");

    auto sqr = engine.CreateScript<int(int)>("(int x) { return x*x; }");
    auto sum = engine.CreateScript<int(int,int)>("(int a, int b) { return a+b; }");

    engine.Recompile();
    std::cout << "sqr(5) = " << sqr(5) << std::endl;
    std::cout << "sum(3,4) = " << sum(3, 4) << std::endl;

    sqr = engine.CreateScript<int(int)>("(int x) { return x-2; }");

    engine.Recompile();
    std::cout << "sqr(5) = " << sqr(5) << std::endl;
    std::cout << "sum(3,4) = " << sum(3, 4) << std::endl;

    return 0;
}