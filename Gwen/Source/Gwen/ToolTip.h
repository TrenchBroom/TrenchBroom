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

using namespace Gwen;
using namespace Gwen::Controls;

namespace ToolTip
{
	void Enable	( Controls::Base* pControl );
	void Disable ( Controls::Base* pControl );

	void ControlDeleted	( Controls::Base* pControl );

	void RenderToolTip	( Skin::Base* skin );
}

#endif
