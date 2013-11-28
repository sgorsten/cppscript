cppscript
=========

This is an experiment in using C++ as a scripting language for a C++ host app. The idea is very simple. You have two classes, script::Function<Sig> and script::Library.

script::Function is a strongly typed function object, callable with the given signature.

script::Library provides mechanisms to generate the aforementioned Function instances from snippets of C++ source code. Specifically, any code snippet which can be inserted after "[]" to form a valid stateless lambda expression can be used. Mechanisms are in place to compile a *.dll at runtime containing the requested functions, as well as to reload a *.dll previously produced by this process. Multiple libraries can be managed and loaded at once, and each can have a separate "preamble", code available to all functions in the library.

This system currently relies on having Visual Studio 2013 installed, and is Windows-specific. However, it is less than 200 lines of code, and should be fairly easy to port to other platforms and toolchains.