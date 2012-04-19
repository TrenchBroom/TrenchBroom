/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#pragma once
#ifndef GWEN_DRAGANDDROP_H
#define GWEN_DRAGANDDROP_H

#include <sstream>

#include "Gwen/Skin.h"
#include "Gwen/Structures.h"

namespace Gwen
{
	namespace DragAndDrop
	{
		extern Package*	CurrentPackage;
		extern Gwen::Controls::Base*	SourceControl;
		extern Gwen::Controls::Base*	HoveredControl;

		bool Start( Gwen::Controls::Base* pControl, Package* pPackage );

		bool OnMouseButton( Gwen::Controls::Base* pHoveredControl, int x, int y, bool bDown );
		void OnMouseMoved( Gwen::Controls::Base* pHoveredControl, int x, int y );

		void RenderOverlay( Gwen::Controls::Canvas* pCanvas, Skin::Base* skin );

		void ControlDeleted( Gwen::Controls::Base* pControl );

        // should be private to the implementation, but declaring them here to silence the compiler
        bool OnDrop( int x, int y );
        bool ShouldStartDraggingControl( int x, int y );
        void UpdateHoveredControl( Gwen::Controls::Base* pCtrl, int x, int y );
	}

}
#endif
