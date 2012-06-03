/*
 GWEN
 Copyright (c) 2010 Facepunch Studios
 See license in Gwen.h
 */


#include "Gwen/Hook.h"

#ifdef GWEN_HOOKSYSTEM

namespace Gwen
{
    namespace Hook
    {
        
        std::list<BaseHook*>	g_HookList;
        
        void AddHook( BaseHook* pHook )
        { 
            g_HookList.push_back( pHook ); 
        }
        
        void RemoveHook( BaseHook* pHook )
        { 
            g_HookList.remove( pHook ); 
        }
        
        HookList& GetHookList()
        {
            return g_HookList;
        }
        
    }
}

#endif