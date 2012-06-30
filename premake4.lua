solution "myrrh"
  configurations { "Debug", "Release" }
  location "build"

  language "C++"
  includedirs { ".", "c:/Utilities/boost/boost_1_49_0" }
  libdirs { "c:/Utilities/boost/boost_1_49_0/stage/lib" }

  configuration { "Debug*" }
    defines { "_DEBUG", "DEBUG" }
    flags   { "Symbols" }
    targetdir "build/Debug"

  configuration { "Release*" }
    defines { "NDEBUG" }
    flags   { "Optimize" }
    targetdir "build/Release"

  project "MyrrhDataTest"
    kind "StaticLib"
    files { "src/data/test/*.hpp", "src/data/test/*.cpp" }

  project "TestMyrrhDataTest"
    kind "ConsoleApp"
    files { "src/data/test/test/*.hpp", "src/data/test/test/*.cpp" }
    links { "MyrrhDataTest" }
    postbuildcommands { "\"$(TargetPath)\"" }
