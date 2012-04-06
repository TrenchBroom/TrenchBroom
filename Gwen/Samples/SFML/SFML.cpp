#include <SFML/Graphics.hpp>
#include <cmath>

#include "Gwen/Renderers/SFML.h"
#include "Gwen/Input/SFML.h"

#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/UnitTest/UnitTest.h"

////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
	// Create the window of the application
	sf::RenderWindow App( sf::VideoMode( 1004, 650, 32 ), "GWEN: SFML", sf::Style::Close );

	Gwen::Renderer::SFML GwenRenderer( App );

	//
	// Create a GWEN skin
	//
	//Gwen::Skin::Simple skin;
	//skin.SetRender( &GwenRenderer );

	Gwen::Skin::TexturedBase skin;
	skin.SetRender( &GwenRenderer );
	skin.Init( "DefaultSkin.png" );

	// The fonts work differently in SFML - it can't use
	// system fonts. So force the skin to use a local one.
	skin.SetDefaultFont( L"OpenSans.ttf", 11 );


	//
	// Create a Canvas (it's root, on which all other GWEN panels are created)
	//
	Gwen::Controls::Canvas* pCanvas = new Gwen::Controls::Canvas( &skin );
	pCanvas->SetSize( App.GetWidth(), App.GetHeight() );
	pCanvas->SetDrawBackground( true );
	pCanvas->SetBackgroundColor( Gwen::Color( 150, 170, 170, 255 ) );


	//
	// Create our unittest control (which is a Window with controls in it)
	//
	UnitTest* pUnit = new UnitTest( pCanvas );
	//pUnit->SetPos( 10, 10 );

	//
	// Create an input processor
	//
	Gwen::Input::SFML GwenInput;
	GwenInput.Initialize( pCanvas );
	
	while ( App.IsOpened() )
	{
		// Handle events
		sf::Event Event;

#if SFML_VERSION_MAJOR == 2
		while ( App.PollEvent(Event) )
#else
		while ( App.GetEvent(Event) )
#endif
		{
			// Window closed or escape key pressed : exit
#if SFML_VERSION_MAJOR == 2
			if ((Event.Type == sf::Event::Closed) || 
				((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Keyboard::Escape)))
#else
			if ((Event.Type == sf::Event::Closed) || 
				((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::Escape)))
#endif
			{
				App.Close();
				break;
			}

			GwenInput.ProcessMessage( Event );
		}

		// Clear the window
		App.Clear();
		
		pCanvas->RenderCanvas();
		
		App.Display();
	}

	return EXIT_SUCCESS;
}
