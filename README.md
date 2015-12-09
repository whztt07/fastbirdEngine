# fastbird engine
fastbird engine is being developed by **fastbird dev studio** creating a sci-fi
game currently. fastbird engine is highly componentized engine that consists of
three layers - **Core**, **Engine** and **Facade/Dedicated** layer. Each layers
has serveral libraries(.lib) and modules(.dll) that can be easily reused for
other applications.

Currently the engine supports Windows. OS X support is planned.

## Modules External libraries
Most modules of fastbird engine is self-contained and do not need external
libraries but the following modules are exceptions and the specified external 
libraries are required to build.

* boost::filesystem - FBFileSystem.dll
* lua 5.2 - FBLua.dll
* zlib - FBRendererD3D11.dll
* openal-soft - FBAudioPlayer.dll
* ALURE - FBAudioPlayer.dll
* libvorbis - FBAudioPlayer.dll
* libogg - FBAudioPlayer.dll, FBVideoPlayer.dll
* libtheora - FBVideoPlayer.dll
* bullet 2.82 - FBPhysics.dll

APIDesign.uml file in the root directory is helpful to check the all of dependencies
in the engine.