solution "myrrh"
  configurations { "Debug", "Release" }
  location "build"

  language "C++"
  includedirs { ".", "c:/Utilities/boost/boost_1_49_0" }
  libdirs { "c:/Utilities/boost/boost_1_49_0/stage/lib" }
  defines { "WIN32" }

  configuration { "Debug*" }
    defines { "_DEBUG", "DEBUG" }
    flags   { "Symbols" }
    targetdir "build/Debug"

  configuration { "Release*" }
    defines { "NDEBUG" }
    flags   { "Optimize" }
    targetdir "build/Release"

  project "MyrrhUtil"
    kind "StaticLib"
    files { "src/util/*.hpp", "src/util/*.cpp" }

  project "MyrrhFile"
    kind "StaticLib"
    files { "src/file/*.hpp", "src/file/*.cpp" }

  project "MyrrhDataTest"
    kind "StaticLib"
    files { "src/data/test/*.hpp", "src/data/test/*.cpp" }

  project "TestMyrrhUtil"
    kind "ConsoleApp"
    files { "src/util/test/*.hpp", "src/util/test/*.cpp" }
    links { "MyrrhUtil", "MyrrhFile", "MyrrhDataTest" }
    postbuildcommands { "\"$(TargetPath)\"" }

  project "TestMyrrhDataTest"
    kind "ConsoleApp"
    files { "src/data/test/test/*.hpp", "src/data/test/test/*.cpp" }
    links { "MyrrhDataTest" }
    postbuildcommands { "\"$(TargetPath)\"" }
