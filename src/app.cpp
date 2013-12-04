#include "cppscript.h"

#include "app_engine/engine.h"

#include <iostream>

int main()
{
    try
    {
        Object player = { "player", { 1, 2, 3 }, { 4, -2, 3 }, 0.5f };
        
        script::Library lib("test", "#include \"../../../../src/app_engine/engine.h\"\n#include <iostream>\n", "../../lib/Debugx64/AppEngine.lib");
        lib.AddVariableReference("player", "Object", &player);
        auto onTimestep = lib.CreateAction("Integrate(player, 0.1f);");
        lib.Recompile(std::cout);

        for (size_t i = 0; i < 3; ++i)
        {
            onTimestep();
            std::cout << "Frame " << i << ": Player is at " << player.position.x << std::endl;
        }

        return 0;
    }
    catch (const std::exception & e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}