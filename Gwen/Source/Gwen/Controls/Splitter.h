/*
 GWEN
 Copyright (c) 2012 Kristian Duske
 See license in Gwen.h
 */

#pragma once
#ifndef TrenchBroom_Splitter_h
#define TrenchBroom_Splitter_h

#include "Gwen/Controls/Base.h"

namespace Gwen {
    namespace Controls {
        class SplitterBar;

		class Splitter : public Gwen::Controls::Base {
        private:
            Gwen::Controls::SplitterBar* m_splitter;
            Gwen::Controls::Base* m_sections[2];
            
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
            Splitter(Gwen::Controls::Base* parent, bool horizontal, int initialPosition);
            
            void Layout(Gwen::Skin::Base* skin);
            
            virtual float CalculateBalance();
            virtual void UpdateSplitter();
            virtual void OnSplitterMoved(Gwen::Controls::Base * control);
            virtual void OnBoundsChanged(Gwen::Rect oldBounds);
            
            virtual void SetPanel(int index, Gwen::Controls::Base* pPanel);
            virtual Gwen::Controls::Base* GetPanel(int index);
            
            virtual void SetMinSize(int index, int minSize);
            virtual void SetMaxSize(int index, int maxSize);
            virtual void SetResize(int index, bool resize);
            
            virtual bool IsZoomed(){ return m_zoomedSection != -1; }
            virtual void Zoom(int index);
            virtual void UnZoom();
            virtual void ZoomChanged();
            virtual void CenterPanels() { m_balance = 0.5f; }
            
            virtual void SetSplitterVisible(bool b);
            virtual void SetSplitterSize(int size);
		};
	}
}
#endif
