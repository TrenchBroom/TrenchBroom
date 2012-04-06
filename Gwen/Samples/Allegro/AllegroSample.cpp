

#include "Gwen/Gwen.h"
#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/UnitTest/UnitTest.h"
#include "Gwen/Input/Allegro.h"
#include "Gwen/Renderers/Allegro.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

int main()
{
	if ( !al_init() ) return -1;

	ALLEGRO_DISPLAY *display = al_create_display( 1024, 768 );
	if ( !display) return -1;

	ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
	if ( !event_queue ) return -1;

	al_init_image_addon();
	al_init_font_addon();
	al_init_primitives_addon();
	al_init_ttf_addon();

	al_install_mouse();
	al_install_keyboard();


	al_register_event_source( event_queue, al_get_display_event_source(display) );
	al_register_event_source( event_queue, al_get_mouse_event_source() );
	al_register_event_source( event_queue, al_get_keyboard_event_source() );

	//
	// Create a GWEN OpenGL Renderer
	//
	Gwen::Renderer::Allegro * pRenderer = new Gwen::Renderer::Allegro();

	//
	// Create a GWEN skin
	//
	Gwen::Skin::TexturedBase skin;
	skin.SetRender( pRenderer );
	skin.Init( "DefaultSkin.png" );

	// The fonts work differently in Allegro - it can't use
	// system fonts. So force the skin to use a local one.
	skin.SetDefaultFont( L"OpenSans.ttf", 11 );

	//
	// Create a Canvas (it's root, on which all other GWEN panels are created)
	//
	Gwen::Controls::Canvas* pCanvas = new Gwen::Controls::Canvas( &skin );
	pCanvas->SetSize( 1024, 768 );
	pCanvas->SetDrawBackground( true );
	pCanvas->SetBackgroundColor( Gwen::Color( 150, 170, 170, 255 ) );

	//
	// Create our unittest control (which is a Window with controls in it)
	//
	UnitTest* pUnit = new UnitTest( pCanvas );
	pUnit->SetPos( 10, 10 );

	//
	// Create a Windows Control helper 
	// (Processes Windows MSG's and fires input at GWEN)
	//
	Gwen::Input::Allegro GwenInput;
	GwenInput.Initialize( pCanvas );


	ALLEGRO_EVENT ev;
	bool bQuit = false;

	while( !bQuit )
	{
		while ( al_get_next_event( event_queue, &ev) ) 
		{
			if ( ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE )
				bQuit = true;

			GwenInput.ProcessMessage( ev );
		}
		
		pCanvas->RenderCanvas();
		al_flip_display();
	}

	al_destroy_display( display );
	al_destroy_event_queue( event_queue );
	return 0;
}