
dofile( os.get() .. ".lua" )

function DefineRenderer( name, filetable )

	project ( "Renderer-"..name )
	files( filetable )
	flags( { "Symbols" } )
	kind( "StaticLib" )
	
	configuration( "Release" )
		targetname( "GWEN-Renderer-"..name )
		
	configuration( "Debug" )
		targetname( "GWEN-Renderer-"..name )

end

function DefineSample( name, filetable, linktable, linktabled, definestable )

	if ( linktabled == nil ) then linktabled = linktable end
	
	project( "Sample-" .. name )
	targetdir ( "../bin" )
	debugdir ( "../bin" )
	if ( definestable ) then defines( definestable ) end
	files { filetable }
	
	kind "WindowedApp"
		
	configuration( "Release" )
		targetname( name .. "Sample" )
		links( linktable )
		
	configuration "Debug"
		targetname( name .. "Sample_D" )
		links( linktabled )

end