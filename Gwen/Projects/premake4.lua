dofile( "inc/include.lua" )

solution "GWEN"

	language "C++"
	location ( os.get() .. "/" .. _ACTION )
	flags { "Unicode", "Symbols", "NoMinimalRebuild", "NoEditAndContinue", "NoPCH", "No64BitChecks", "NoRTTI" }
	targetdir ( "../lib/" .. os.get() )
	libdirs { "../lib/", "../lib/" .. os.get() }

	configurations
	{ 
		"Release",
		"Debug"
	}
	
	if ( _ACTION == "vs2010" or _ACTION=="vs2008" ) then
		buildoptions { "/MP"  }
	end 

configuration "Release"
	defines { "NDEBUG" }
	flags{ "Optimize", "FloatFast" }
	includedirs { "../include/" }
	
configuration "Debug"
	defines { "_DEBUG" }
	includedirs { "../include/" }

project "GWEN DLL"
	defines { "GWEN_COMPILE_DLL" }
	files { "../src/**.*", "../include/Gwen/**.*" }
	kind "SharedLib"
	
	configuration "Release"
		targetname( "gwen" )
		
	configuration "Debug"
		targetname( "gwend" )

project "GWEN Static"
	defines { "GWEN_COMPILE_STATIC" }
	files { "../src/**.*", "../include/Gwen/**.*" }
	flags { "Symbols" }
	kind "StaticLib"
	
	configuration "Release"
		targetname( "gwen_static" )
		
	configuration "Debug"
		targetname( "gwend_static" )
		
project "UnitTest"
	files { "../UnitTest/**.*" }
	flags { "Symbols" }
	kind "StaticLib"
	
	configuration "Release"
		targetname( "unittest" )
		
	configuration "Debug"
		targetname( "unittestd" )
		

--
-- Renderers
--

DefineRenderer( "OpenGL", {"../Renderers/OpenGL/OpenGL.cpp"} )
DefineRenderer( "OpenGL_DebugFont", { "../Renderers/OpenGL/OpenGL.cpp", "../Renderers/OpenGL/DebugFont/OpenGL_DebugFont.cpp" } )
DefineRenderer( "SFML", { "../Renderers/SFML/SFML.cpp" } )
DefineRenderer( "Allegro", { "../Renderers/Allegro/Allegro.cpp" } )

if ( os.get() == "windows" ) then
	DefineRenderer( "DirectX9", { "../Renderers/DirectX9/DirectX9.cpp" } )
	DefineRenderer( "GDI", { "../Renderers/GDIPlus/GDIPlus.cpp", "../Renderers/GDIPlus/GDIPlusBuffered.cpp" } )
end

--
-- Samples
--

DefineSample( "SFML", { "../Samples/SFML/SFML.cpp" }, SFML_LIBS, SFML_LIBS_D )
DefineSample( "Allegro", { "../Samples/Allegro/AllegroSample.cpp" }, ALLEGRO_LIBS, ALLEGRO_LIBS_D )

if ( os.get() == "windows" ) then

	DefineSample( "DirectX9", { "../Samples/Direct3D/Direct3DSample.cpp" }, { "Renderer-DirectX9", "GWEN Static", "UnitTest" } )
	DefineSample( "WindowsGDI", { "../Samples/WindowsGDI/WindowsGDI.cpp" }, { "Renderer-GDI", "GWEN Static", "UnitTest" } )
	DefineSample( "OpenGL", { "../Samples/OpenGL/OpenGLSample.cpp" }, { "Renderer-OpenGL", "GWEN Static", "UnitTest", "opengl32", "FreeImage" } )
	DefineSample( "OpenGL_DebugFont", { "../Samples/OpenGL/OpenGLSample.cpp" }, { "Renderer-OpenGL_DebugFont", "GWEN Static", "UnitTest", "opengl32", "FreeImage" }, nil, { "USE_DEBUG_FONT" } )

end


