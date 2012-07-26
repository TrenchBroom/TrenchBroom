/*
 GWEN
 Copyright (c) 2010 Facepunch Studios
 See license in Gwen.h
 */


#include "Gwen/Gwen.h"
#include "Gwen/Controls/TextBox.h"
#include "Gwen/Skin.h"
#include "Gwen/Utility.h"
#include "Gwen/Platform.h"

namespace Gwen
{
    namespace Controls
    {
        
        GWEN_CONTROL_CONSTRUCTOR( TextBoxNumeric )
        {
            SetText( L"0" );
        }
        
        bool TextBoxNumeric::IsTextAllowed( const Gwen::UnicodeString& str, int iPos )
        {
            if ( str.length() == 0 )
                return true;
            
            bool hasMinus = false;
            bool hasDot = false;
            
            for (size_t i=0; i<str.length(); i++)
            {
                if ( str[i] == L'-' )
                {
                    // Has to be at the start
                    if ( i != 0 || iPos != 0 )
                        return false;
                    
                    // Can only be one
                    if ( hasMinus )
                        return false;
                    
                    hasMinus = true;
                    continue;
                }
                
                if ( str[i] == L'0' ) continue;
                if ( str[i] == L'1' ) continue;
                if ( str[i] == L'2' ) continue;
                if ( str[i] == L'3' ) continue;
                if ( str[i] == L'4' ) continue;
                if ( str[i] == L'5' ) continue;
                if ( str[i] == L'6' ) continue;
                if ( str[i] == L'7' ) continue;
                if ( str[i] == L'8' ) continue;
                if ( str[i] == L'9' ) continue;
                
                if ( str[i] == L'.' )
                {
                    // Can't be at the start or the end
                    if (i == 0 || i == str.length() - 1)
                        return false;
                    
                    // Already a dot
                    if ( hasDot )
                        return false;
                    
                    hasDot = true;
                    continue;
                }
                
                return false;
            }
            
            return true;
        }
        
        float TextBoxNumeric::GetFloatFromText()
        {
            double temp = GwenUtil_WideStringToFloat( GetText().c_str() );
            return temp;
        }
        
    }
}