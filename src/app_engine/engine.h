#ifndef ENGINE_H
#define ENGINE_H

#include <cstdlib>
#include <cassert>
#include <cmath>

#include <functional>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <map>

struct float3 { float x,y,z; };
struct Object { std::string name; float3 position,velocity; float size; };
void Integrate(Object & obj, float timestep);

#endif