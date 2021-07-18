# Godot Physics Performance Test
This is a project that was used to test three physics backends for Godot.
The test was setup as follows:
* A static floor
* Four static walls with 20 units of spacing
* Spawn rigid body unit cubes until the frames per second drop below 60
* Additionally spawn cubes as scenes without physics
* Additionally spawn cubes as instanced meshes and wihtout physics
* Compare the results

The physics backends used were:
* Godot's built in physics
* Godot's bullet physics integration
* A quick and dirty, partial implementation of PhysX written by me

Additionally the test used three different methods of spawning rigid body cubes
when using the bullet or built in phyiscs backends:
* Instantiating a scene with a `RigidBody`, `MeshInstance` and `CollisionShape` node
* Using a C++ `NativeScript` to directly interface with the `VisualServer` and `PhysicsServer`
* Using GDScript to directly interface with the `VisualServer` and `PhysicsServer`

System used:
* Core i7 10700K @stock
* Geforce RTX 2080 Super
* 16 GB of RAM

# Purpose
I saw some videos doing the same type of cube spawning. None of them
mentioned that the limiting factor is the physics engine and not Godot itself. 
For new people this could lead to the conclusion that Godot is generally slow,
which is not the case. While Godot has many areas where performance is subpar
compared to other engines, it is not as slow as people tend to think.

# Results

## Best case results

| Backend | Number of cubes |
|----------|----------------|
| Built in | 1161 |
| Bullet | 1207 |
| Built in 4.0 [1] [2] | 1325 |
| PhysX | 3952 |

[1]: Simulation unstable, cubes collabsed into themselves

[2]: Using commit 51b0aed4b722407c02c7685cefe6b403df7e2806

## Nodes vs. Servers

### Bullet backend

| Method | Number of cubes |
|--------|------|
| Nodes | 1 160 |
| Servers (C++) | 1 207 |
| Servers (GDScript) | 1 151 |

### Built in backend
| Method | Number of cubes |
|-|-|
| Nodes [1] | 939 |
| Servers (C++) | 1 161 |
| Servers (GDNative) |  1 123 |

[1]: Simulation unstable, cubes collabsed into themselves

## Physics vs. no physics

| Method | Number of cubes |
|-|-|
| Nodes (no physics) | 12 660 |
| Instanced meshes (no physics) | 518 400

# Evaluation
When considering physics, performance is largely dependent on the backend that is used. 
The proof of concept implementation of PhysX was about 3.27 times faster than
the bullet integration, which in turn was about 1.04 times faster that the built in physics backend.
Godot 4.0's built in backend is able to beat the current bullet backend by a factor of about 1.1. 

As Godot's documentation mentions, usings servers directly instead of relying on the scene tree system
does improve performance. However with the factors being merely about 1.04 for the bullet backend, one 
should consider doing this step carefully, since not using the scene tree means losing a lot of 
flexibility. With the built in physics the factor is larger (about 1.24), but again the tradeoff between
performance and flexibility should be considered carefully.

When looking at working with servers either through GDScript or C++ (GDNative), the differences
become mostly irrelevant (~1.05 and ~1.03, favoring C++). This makes sense, since both the GDScript and
C++ implementations merely call engine methods and rarely do any calculations at all. Sticking
with GDScript for this sort of work is certainly the easier option.

An interesting outlier is the fact that the simualtion using nodes and the built in backend seem to 
be unstable, as the tests in Godot 3.3.2 and Godot 4.0 resulted in the cubes collabsing into themselves.
However when using servers instead of nodes the simulation remained stable.

As predicted for this test, Godot's performance is limited by the amount of physics objects used. 
Once physics are ignored, Godot's scene tree can handle more that 10 000 objects. Again not
using the scene tree and instead using GPU instancing improves performance
further and enables the rendering of more than 500 000 cubes. 

# Conclusion
The bottom line is, that these sort of benchmarks are pretty useless. Also keep in mind, that the
results largely depend on the amount of contacts made by physics objects. In an [earlier test](https://youtu.be/t9KGC1otGc0) I didn't use a walled of area for spawning, which lead to the cubes spreading horizontally, which in turn lead to PhysX being able to handle 4 999 cubes (a gain of over 1000 cubes). As a result it makes much more sense to test a real scenario of a game rather than to rely on these benchmarks. Most games won't reach that many physical moving objects anyway. If, however, the game is supposed to handle thousands of moving physical objects (such as maybe bullet hell games) careful evaluation is a must.

# Running the Project
For your convenience, 64bit Windows DLLs are supplied in `NativeLibraryImpl/bin`. Place all DLLs next to your Godot executable. Then the project should launch fine.

# Building the project on windows
You need to build the PhysX SDK first. Make sure to change one of the SDK presets to generate dynamically
linked binaries by setting `NV_USE_STATIC_WINCRT` to `False`. 

Make sure you cloned this repo recursively, as it relies on godot-cpp which in turn relies on godot-headers.

Navigate to `NativeLibraryImpl/godot-cpp` and run `scons platform=windows target=release generate_bindings=yes`.

Open `NativeLibraryImpl/compile.bat` and change the paths of the PhysX SDK to match your installation. 
Now open the Visual Studio Command Prompt, navigate to the `NativeLibraryImpl` folder and run `compile.bat`. 

Unfortunately I cannot provide support for other platforms.

# Third Party Licenses
## PhysX SDK
    Copyright (c) 2008-2021 NVIDIA Corporation. All rights reserved. Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    Neither the name of NVIDIA CORPORATION nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## Kenney.nl textures

	Prototype Textures 1.0

	Created/distributed by Kenney (www.kenney.nl)
	Creation date: 08-04-2020

			------------------------------

	License: (Creative Commons Zero, CC0)
	http://creativecommons.org/publicdomain/zero/1.0/

	This content is free to use in personal, educational and commercial projects.
	Support us by crediting Kenney or www.kenney.nl (this is not mandatory)

			------------------------------

	Donate:   http://support.kenney.nl
	Request:  http://request.kenney.nl
	Patreon:  http://patreon.com/kenney/

	Follow on Twitter for updates:
	http://twitter.com/KenneyNL








