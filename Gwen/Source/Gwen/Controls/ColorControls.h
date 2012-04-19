/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/
 
#pragma once
#ifndef GWEN_CONTROLS_COLORCONTROLS_H
#define GWEN_CONTROLS_COLORCONTROLS_H

#include "Gwen/Controls/Base.h"
#include "Gwen/Gwen.h"
#include "Gwen/Skin.h"

static Gwen::Color HSVToColor( float h, float s, float v );
static Gwen::HSV RGBtoHSV( int r, int g, int b );
static Gwen::Color LerpColor( Gwen::Color &toColor, Gwen::Color &fromColor, float amount );

namespace Gwen 
{
	namespace Controls
	{

		class ColorLerpBox : public Controls::Base
		{
			public:
				GWEN_CONTROL( ColorLerpBox, Controls::Base );
				virtual void Render( Gwen::Skin::Base* skin );
				Gwen::Color GetColorAtPos(int x, int y );
				void SetColor( Gwen::Color color, bool onlyHue = true );
				virtual void OnMouseMoved( int x, int y, int deltaX, int deltaY );
				virtual void OnMouseClickLeft( int x, int y, bool bDown );
				Gwen::Color GetSelectedColor();

				Event::Caller	onSelectionChanged;
			protected:
				Gwen::Point cursorPos;
				bool m_bDepressed;
				int m_Hue;
				
		};

		class ColorSlider : public Controls::Base
		{
			public:
				GWEN_CONTROL( ColorSlider, Controls::Base );
				virtual void Render( Gwen::Skin::Base* skin );
				virtual void OnMouseMoved( int x, int y, int deltaX, int deltaY );
				virtual void OnMouseClickLeft( int x, int y, bool bDown );
				Gwen::Color GetSelectedColor();
				Gwen::Color GetColorAtHeight(int y );
				void SetColor( Gwen::Color color );

				Event::Caller	onSelectionChanged;

			protected:
				int m_iSelectedDist;
				bool m_bDepressed;

		};
	}

}
#endif
