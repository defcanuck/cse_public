# cse_public
## CSEngine
This is the low-level layer of my personal game engine, written C++ and GLSL.  This is not provided to the public for use (there are several missing dependencies needed to compile), but merely as a showcase of my programming style in C++.

## Highlights
- JSON-like text serialization and instant reflection for common data types and objects (see ClassDef.h if you think nested preprocessors are beautiful)
- Cross platform renderer (OpenGL on Windows, OpenGL ES/Metal on iOS)
- Physics via Box2D
- Reflection via LuaBind
- Draw call batching and texture atlases
- Animated vector sprites via Spine
- Post-processing support for motion blur, chromatic abberation
- Custom UI system with agnostic input 
- Entity Component System
- Custom Shaders via GLSL/Metal
- Custom, efficient particle system with heap batching
- Profiler Scopes for real-time performance monitoring
- GC-esque memory/object management via std::shared_ptr
- 3D math via GLM
- lots of other magical things I've picked up over the years

## TODO
NOTE:  There's some weird formatting that snuck in somehow, probably from me opening this on another macbook pro without my tabs properly set.  Will have to sync this with the current engine version with all that grossness cleaned up.  
