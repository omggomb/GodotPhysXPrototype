@echo off

rem Run from Visual Studio Command Prompt!

cl /O2 /Fosrc/init.obj /c src/init.cpp /nologo -EHsc -DNDEBUG /MD /Igodot-cpp\include /Igodot-cpp\include\core /Igodot-cpp\include\gen /Igodot-cpp\godot-headers /ID:\Dev\Software\PhysX\physx\include /ID:\Dev\Software\PhysX\pxshared\include
link /nologo /dll /out:bin\libsimple.dll /LIBPATH:D:\Dev\Software\PhysX\physx\bin\win.x86_64.vc142.md\release /implib:bin\libsimple.lib src\init.obj godot-cpp\bin\libgodot-cpp.windows.release.64.lib PhysX_64.lib PhysXCommon_64.lib PhysXFoundation_64.lib PhysXExtensions_static_64.lib PhysXPvdSDK_static_64.lib