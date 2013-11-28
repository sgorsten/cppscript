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

int main()
{
    ScriptedFunction<int(int)> sqr;
    sqr.source = "(int x) { return x*x; }";

    // Write function to script.cpp
    std::ofstream out("script.cpp");
    out << "extern \"C\" __declspec(dllexport) int (*__get_script_function())(int) { return []" << sqr.source << "; }";
    out.close();

    // Compile the script
    FILE * pipe = _popen("compile.bat", "r");
    char buffer[1024]; 
    while (fgets(buffer, sizeof(buffer), pipe))
    {
        std::cout << buffer << std::endl;
    }

    // Load DLL and implementation of function
    HMODULE hmod = LoadLibrary(L"..\\Debug\\dll.dll");
    auto loader = (int(*(*)())(int))GetProcAddress(hmod, "__get_script_function");
    sqr.impl = loader();

    // Call function
    std::cout << "sqr(5) = " << sqr.impl(5) << std::endl;

    // Unload DLL
    sqr.impl = nullptr;
    FreeLibrary(hmod);

    return 0;
}