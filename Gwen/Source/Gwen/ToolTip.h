/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#pragma once
#ifndef GWEN_TOOLTIP_H
#define GWEN_TOOLTIP_H

#include "Gwen/Controls/Base.h"
#include "Gwen/Gwen.h"
#include "Gwen/Skin.h"

namespace ToolTip
{
	void Enable	( Gwen::Controls::Base* pControl );
	void Disable ( Gwen::Controls::Base* pControl );

	void ControlDeleted	( Gwen::Controls::Base* pControl );

	void RenderToolTip	( Gwen::Skin::Base* skin );
}

#endif
