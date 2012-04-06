/*
	GWEN
	Copyright (c) 2011 Facepunch Studios
	See license in Gwen.h
*/

#pragma once
#ifndef GWEN_INPUT_ALLEGRO_H
#define GWEN_INPUT_ALLEGRO_H

#include "Gwen/InputHandler.h"
#include "Gwen/Gwen.h"
#include "Gwen/Controls/Canvas.h"

#include <allegro5/allegro.h>

namespace Gwen
{
	namespace Input
	{
		class Allegro
		{
			public:

				Allegro()
				{
					m_Canvas = NULL;
					m_MouseX = 0;
					m_MouseY = 0;
				}

				void Initialize( Gwen::Controls::Canvas* c )
				{
					m_Canvas = c;
				}

				unsigned char TranslateKeyCode( int iKeyCode )
				{
					switch ( iKeyCode )
					{
						case ALLEGRO_KEY_BACKSPACE:	return Gwen::Key::Backspace;
						case ALLEGRO_KEY_ENTER:		return Gwen::Key::Return;
						case ALLEGRO_KEY_ESCAPE:	return Gwen::Key::Escape;
						case ALLEGRO_KEY_TAB:		return Gwen::Key::Tab;
						case ALLEGRO_KEY_SPACE:		return Gwen::Key::Space;
						case ALLEGRO_KEY_UP:		return Gwen::Key::Up;
						case ALLEGRO_KEY_DOWN:		return Gwen::Key::Down;
						case ALLEGRO_KEY_LEFT:		return Gwen::Key::Left;
						case ALLEGRO_KEY_RIGHT:		return Gwen::Key::Right;
						case ALLEGRO_KEY_HOME:		return Gwen::Key::Home;
						case ALLEGRO_KEY_END:		return Gwen::Key::End;
						case ALLEGRO_KEY_DELETE:	return Gwen::Key::Delete;
						case ALLEGRO_KEY_LCTRL:		return Gwen::Key::Control;
						case ALLEGRO_KEY_ALT:		return Gwen::Key::Alt;
						case ALLEGRO_KEY_LSHIFT:	return Gwen::Key::Shift;
						case ALLEGRO_KEY_RCTRL:		return Gwen::Key::Control;
						case ALLEGRO_KEY_ALTGR:		return Gwen::Key::Alt;
						case ALLEGRO_KEY_RSHIFT:	return Gwen::Key::Shift;
					}

					return Gwen::Key::Invalid;
				}

				bool ProcessMessage( ALLEGRO_EVENT& event )
				{
					if ( !m_Canvas ) return false;

					switch ( event.type )
					{
	
						case ALLEGRO_EVENT_MOUSE_AXES:
							{
								int dx = event.mouse.x - m_MouseX;
								int dy = event.mouse.y - m_MouseY;

								m_MouseX = event.mouse.x;
								m_MouseY = event.mouse.y;

								return m_Canvas->InputMouseMoved( m_MouseX, m_MouseY, dx, dy );
							}

	
						case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
						case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
							{
								return m_Canvas->InputMouseButton( event.mouse.button-1, event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN );
							}
							
/*
	TODO!
						case sf::Event::MouseWheelMoved:
							{
								return m_Canvas->InputMouseWheel( event.MouseWheel.Delta * 60 );
							}
*/

						case ALLEGRO_EVENT_KEY_CHAR:
							{
								return m_Canvas->InputCharacter( event.keyboard.unichar );
							}

						case ALLEGRO_EVENT_KEY_DOWN:
						case ALLEGRO_EVENT_KEY_UP:
							{
								bool bPressed = (event.type == ALLEGRO_EVENT_KEY_DOWN);

								if ( event.keyboard.keycode && bPressed && event.keyboard.keycode >= 'a' && event.keyboard.keycode <= 'z' )
								{
									return m_Canvas->InputCharacter( event.keyboard.keycode );
								}

								unsigned char iKey = TranslateKeyCode( event.keyboard.keycode );

								return m_Canvas->InputKey( iKey, bPressed );

							}
					}

					return false;
				}

				protected:

					Gwen::Controls::Canvas*	m_Canvas;
					int m_MouseX;
					int m_MouseY;

		};
	}
}
#endif
