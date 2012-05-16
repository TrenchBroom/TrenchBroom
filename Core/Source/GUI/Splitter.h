/*
 Copyright (C) 2010-2012 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef TrenchBroom_Splitter_h
#define TrenchBroom_Splitter_h

#include "Gwen/Gwen.h"
#include "Gwen/Controls/Base.h"
#include "Gwen/Controls/SplitterBar.h"

using namespace Gwen;

namespace TrenchBroom 
{
	namespace Gui
	{
		class Splitter : public Controls::Base {
        private:
            Controls::SplitterBar* m_splitter;
            Controls::Base* m_sections[2];
            
            bool m_horizontal;
            float m_balance;
            int m_barSize;
            int m_minSize[2];
            int m_maxSize[2];
            bool m_resize[2];
            int m_initialPosition;
            char m_zoomedSection;
            
            Gwen::Event::Caller	onZoomed;
            Gwen::Event::Caller	onUnZoomed;
            Gwen::Event::Caller	onZoomChange;
        public:
            Splitter(Controls::Base* parent, bool horizontal, int initialPosition);
            
            void Layout( Skin::Base* skin );
            
            virtual float CalculateBalance();
            virtual void UpdateSplitter();
            virtual void OnSplitterMoved(Controls::Base * control);
            virtual void OnBoundsChanged(Gwen::Rect oldBounds);
            
            virtual void SetPanel(int index, Controls::Base* pPanel);
            virtual Controls::Base* GetPanel(int index);
            
            virtual void SetMinSize(int index, int minSize);
            virtual void SetMaxSize(int index, int maxSize);
            virtual void SetResize(int index, bool resize);
            
            virtual bool IsZoomed(){ return m_zoomedSection != -1; }
            virtual void Zoom(int index);
            virtual void UnZoom();
            virtual void ZoomChanged();
            virtual void CenterPanels() { m_balance = 0.5f; }
            
            virtual void SetSplitterVisible(bool b){ m_splitter->SetShouldDrawBackground( b ); }
            virtual void SetSplitterSize(int size) { m_barSize = size; }
		};
	}
}
#endif
