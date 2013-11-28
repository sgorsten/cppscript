#include "cppscript.h"

#include <iostream>

int main()
{
    script::Library lib("test", "#include <iostream>\n");

    lib.DefineSignature<void()>("void", "");
    lib.DefineSignature<int(int)>("int", "int");
    lib.DefineSignature<double(double, double)>("double", "double,double");

    auto hello = lib.CreateScript<void()>("() { std::cout << \"Hello world!\" << std::endl; }");
    auto hello2 = lib.CreateScript<void()>("() { std::cout << \"Hello world!\" << std::endl; }");
    lib.Recompile(std::cout);
    hello(); hello2();

    hello = lib.CreateScript<void()>("() { std::cout << \"Scripting in C++ is surprisingly simple!\" << std::endl; }");
    auto sqr = lib.CreateScript<int(int)>("(int x) { return x*x; }");
    auto sum = lib.CreateScript<double(double, double)>("(double a, double b) { return a+b; }");
    lib.Recompile(std::cout);

    // Demonstrate loading library without recompilation
    lib.Unload();
    lib.Load();

    std::cout << "sqr(5) = " << sqr(5) << std::endl;
    std::cout << "sum(3.1,4.2) = " << sum(3.1, 4.2) << std::endl;
    hello();

    return 0;
}