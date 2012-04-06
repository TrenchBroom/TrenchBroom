/*
	GWEN
	Copyright (c) 2011 Facepunch Studios
	See license in Gwen.h
*/


#pragma once
#ifndef GWEN_UNITTEST_UNITTEST_H
#define GWEN_UNITTEST_UNITTEST_H

#include "Gwen/Gwen.h"
#include "Gwen/Align.h"
#include "Gwen/Utility.h"
#include "Gwen/Controls/WindowControl.h"
#include "Gwen/Controls/TabControl.h"
#include "Gwen/Controls/ListBox.h"
#include "Gwen/Controls/DockBase.h"
#include "Gwen/Controls/StatusBar.h"

class UnitTest;

#ifdef _MSC_VER
#	ifdef NDEBUG
#		pragma comment ( lib, "unittest.lib" )
#	else
#		pragma comment ( lib, "unittestd.lib" )
#	endif
#endif


class GUnit : public Gwen::Controls::Base
{
	public:

		GWEN_CONTROL_INLINE( GUnit, Gwen::Controls::Base )
		{
			m_pUnitTest = NULL;
		}

		void SetUnitTest( UnitTest* u ){ m_pUnitTest = u; }

		void UnitPrint( const Gwen::UnicodeString& str );
		void UnitPrint( const Gwen::String& str );

		void Layout( Gwen::Skin::Base* skin )
		{
			SizeToChildren( true, true );
		}


		UnitTest* m_pUnitTest;
};

class UnitTest : public Gwen::Controls::DockBase
{
	public:

		GWEN_CONTROL( UnitTest, Gwen::Controls::DockBase );

		void PrintText( const Gwen::UnicodeString& str );

		void Render( Gwen::Skin::Base* skin );


	private:

		void OnCategorySelect( Gwen::Controls::Base* pBase, Gwen::Controls::Base* pControl );

		Gwen::Controls::TabControl*	m_TabControl;
		Gwen::Controls::ListBox*	m_TextOutput;
		Gwen::Controls::StatusBar*	m_StatusBar;
		unsigned int				m_iFrames;
		float						m_fLastSecond;

		Gwen::Controls::Base*		m_pLastControl;

};

#define DEFINE_UNIT_TEST( name, displayname ) GUnit* RegisterUnitTest_##name( Gwen::Controls::Base* tab ){ return new name( tab ); }
#endif
