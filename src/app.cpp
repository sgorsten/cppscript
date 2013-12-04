#include "cppscript.h"

#include <iostream>

int main()
{
    try
    {
        int count = 0;
        script::Library lib("test", "#include <iostream>\n#include <string>\n");
        lib.AddVariableReference("count", "int", &count);

        lib.DefineSignature<void()>("void()");
        lib.DefineSignature<int(int)>("int(int)");
        lib.DefineSignature<double(double, double)>("double(double,double)");
        lib.DefineSignature<void(std::string)>("void(std::string)");

        auto print = lib.CreateScript<void(std::string)>("(std::string s) { std::cout << s << std::endl; }");

        auto hello = lib.CreateScript<void()>("() { std::cout << \"Hello world for the \" << ++count << \"th time!\" << std::endl; }");
        lib.Recompile(std::cout);
        hello();
        hello();
        hello();

        print("Woot!");

        auto sqr = lib.CreateScript<int(int)>("(int x) { return x*x; }");
        auto sum = lib.CreateScript<double(double, double)>("(double a, double b) { return a+b; }");
        lib.Recompile(std::cout);

        std::cout << "sqr(5) = " << sqr(5) << std::endl;
        std::cout << "sum(3.1,4.2) = " << sum(3.1, 4.2) << std::endl;
        hello();
        return 0;
    }
    catch (const std::exception & e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}