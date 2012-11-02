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

#ifndef __TrenchBroom__InputControllerFeedbackFigure__
#define __TrenchBroom__InputControllerFeedbackFigure__

#include "Renderer/Figure.h"

#include <cassert>
#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Controller {
        class Tool;
    }
    
    namespace Renderer {
        class InputControllerFeedbackFigure : public Figure {
        private:
            typedef std::vector<Figure*> FigureList;
            typedef std::map<Controller::Tool*, FigureList> FigureMap;
            
            FigureMap m_figures;
            FigureList m_deleteFigures;
            
            Controller::Tool* m_singleFeedbackProvider;
        public:
            InputControllerFeedbackFigure();
            ~InputControllerFeedbackFigure();
            
            inline Controller::Tool* singleFeedbackProvider() const {
                return m_singleFeedbackProvider;
            }
            
            inline void setSingleFeedbackProvider(Controller::Tool* singleFeedbackProvider) {
                m_singleFeedbackProvider = singleFeedbackProvider;
            }
            
            void addFigure(Controller::Tool* tool, Figure* figure);
            void removeFigure(Controller::Tool* tool, Figure* figure);
            void deleteFigure(Controller::Tool* tool, Figure* figure);

            void render(Vbo& vbo, RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__InputControllerFeedbackFigure__) */
