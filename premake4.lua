solution "AnimusSolution"
   configurations { "Debug", "Release" }
   platforms {"native", "x64", "x32"}

   -- Animus
   project "Animus"
      kind "ConsoleApp"
      language "C++"
      files { "src/animus.cpp"}
      includedirs { "lib/glfw/include", "lib/fmod/inc", "src", "common", "lib/"}
      links {"glfw", "glew", "stb", "imgui"}
      libdirs {"lib/**"}
      defines { "GLEW_STATIC" }
     
      configuration { "linux" }
         links {"X11","Xrandr", "rt", "GL", "GLU", "pthread", "fmodex64"}
       
      configuration { "windows" }
         links {"glu32","opengl32", "gdi32", "winmm", "user32"}

      configuration { "macosx" }
         linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit" }
       
      configuration "Debug"
         defines { "DEBUG" }
         flags {"ExtraWarnings", "Symbols" }
         targetsuffix "d"

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize"}    
         targetsuffix "r"

   -- Spectrum
   project "Spectrum"
      kind "ConsoleApp"
      language "C++"
      files { "src/spectrum.cpp"}
      includedirs { "lib/glfw/include", "lib/fmod/inc", "src", "common", "lib/"}
      links {"glfw", "glew", "stb", "imgui"}
      libdirs {"lib/**"}
      defines { "GLEW_STATIC" }
     
      configuration { "linux" }
         links {"X11","Xrandr", "rt", "GL", "GLU", "pthread", "fmodex64"}
       
      configuration { "windows" }
         links {"glu32","opengl32", "gdi32", "winmm", "user32"}

      configuration { "macosx" }
         linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit" }
       
      configuration "Debug"
         defines { "DEBUG" }
         flags {"ExtraWarnings", "Symbols" }
         targetsuffix "d"

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize"}    
         targetsuffix "r"

   -- GLFW Library
   project "glfw"
      kind "StaticLib"
      language "C"
      files { "lib/glfw/lib/*.h", "lib/glfw/lib/*.c", "lib/glfw/include/GL/glfw.h" }
      includedirs { "lib/glfw/lib", "lib/glfw/include"}

      configuration {"linux"}
         files { "lib/glfw/lib/x11/*.c", "lib/glfw/x11/*.h" }
         includedirs { "lib/glfw/lib/x11" }
         defines { "_GLFW_USE_LINUX_JOYSTICKS", "_GLFW_HAS_XRANDR", "_GLFW_HAS_PTHREAD" ,"_GLFW_HAS_SCHED_YIELD", "_GLFW_HAS_GLXGETPROCADDRESS" }
         buildoptions { "-pthread" }

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
         targetdir "bin/debug"

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }    
         targetdir "bin/release"

   -- GLEW Library         
   project "glew"
      kind "StaticLib"
      language "C"
      files {"lib/glew/*.c", "lib/glew/*.h"}
      defines { "GLEW_STATIC" }

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
         targetdir "bin/debug"

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }    
         targetdir "bin/release"

   -- IMGUI library         
   project "imgui"
      kind "StaticLib"
      language "C"
      files {"lib/imgui/*.cpp", "lib/imgui/*.h"}
      includedirs { "lib/" }

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
         targetdir "bin/debug"

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }    
         targetdir "bin/release"

   -- stb Library         
   project "stb"
      kind "StaticLib"
      language "C"
      files {"lib/stb/*.c", "lib/stb/*.h"}

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
         targetdir "bin/debug"

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }    
         targetdir "bin/release"
