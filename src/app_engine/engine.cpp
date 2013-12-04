#include "engine.h"

void Integrate(Object & obj, float timestep)
{
    obj.position.x += obj.velocity.x * timestep;
    obj.position.y += obj.velocity.y * timestep;
    obj.position.z += obj.velocity.z * timestep;
}