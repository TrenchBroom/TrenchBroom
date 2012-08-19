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
        
        bool TextBoxNumeric::IsTextAllowed( const Gwen::UnicodeString& insertStr, int iPos )
        {
            if ( insertStr.length() == 0 )
                return true;

            UnicodeString content = GetText();
            content.insert(iPos, insertStr);
            
            bool hasMinus = false;
            bool hasDot = false;
            
            for (size_t i=0;  i <content.length(); i++)
            {
                if ( content[i] == L'-' )
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
                
                if ( content[i] == L'0' ) continue;
                if ( content[i] == L'1' ) continue;
                if ( content[i] == L'2' ) continue;
                if ( content[i] == L'3' ) continue;
                if ( content[i] == L'4' ) continue;
                if ( content[i] == L'5' ) continue;
                if ( content[i] == L'6' ) continue;
                if ( content[i] == L'7' ) continue;
                if ( content[i] == L'8' ) continue;
                if ( content[i] == L'9' ) continue;
                
                if ( content[i] == L'.' )
                {
                    // Can't be at the start
                    if (i == 0)
                        return false;
                    
                    // Can't be at the end either unless it was just inserted there
                    if (i == content.length() - 1 && static_cast<int>(i) != iPos)
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