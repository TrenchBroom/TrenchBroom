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

#include "InputControllerFeedbackFigure.h"

namespace TrenchBroom {
    namespace Renderer {
        InputControllerFeedbackFigure::InputControllerFeedbackFigure() :
        m_singleFeedbackProvider(NULL) {}
        
        InputControllerFeedbackFigure::~InputControllerFeedbackFigure() {
            while (!m_deleteFigures.empty()) delete m_deleteFigures.back(), m_deleteFigures.pop_back();
            
            FigureMap::iterator it, end;
            for (it = m_figures.begin(), end = m_figures.end(); it != end; ++it) {
                FigureList& figureList = it->second;
                while (!figureList.empty()) delete figureList.back(), figureList.pop_back();
            }
            m_figures.clear();
        }

        void InputControllerFeedbackFigure::addFigure(Controller::Tool* tool, Figure* figure)  {
            assert(tool != NULL);
            assert(figure != NULL);
            
            m_figures[tool].push_back(figure);
        }
        
        void InputControllerFeedbackFigure::removeFigure(Controller::Tool* tool, Figure* figure)  {
            assert(tool != NULL);
            assert(figure != NULL);
            
            FigureMap::iterator it = m_figures.find(tool);
            if (it == m_figures.end())
                return;
            
            FigureList& figureList = it->second;
            figureList.erase(std::remove(figureList.begin(), figureList.end(), figure), figureList.end());
            if (figureList.empty())
                m_figures.erase(it);
        }
        
        void InputControllerFeedbackFigure::deleteFigure(Controller::Tool* tool, Figure* figure)  {
            assert(tool != NULL);
            assert(figure != NULL);
            
            removeFigure(tool, figure);
            m_deleteFigures.push_back(figure);
        }

        void InputControllerFeedbackFigure::render(Vbo& vbo, RenderContext& context) {
            while (!m_deleteFigures.empty()) delete m_deleteFigures.back(), m_deleteFigures.pop_back();
            
            if (m_singleFeedbackProvider != NULL) {
                FigureMap::iterator mapIt = m_figures.find(m_singleFeedbackProvider);
                if (mapIt != m_figures.end()) {
                    FigureList& figureList = mapIt->second;
                    FigureList::iterator figureIt, figureEnd;
                    for (figureIt = figureList.begin(), figureEnd = figureList.end(); figureIt != figureEnd; ++figureIt) {
                        Figure& figure = **figureIt;
                        figure.render(vbo, context);
                    }
                }
            } else {
                FigureMap::iterator mapIt, mapEnd;
                for (mapIt = m_figures.begin(), mapEnd = m_figures.end(); mapIt != mapEnd; ++mapIt) {
                    FigureList& figureList = mapIt->second;
                    FigureList::iterator figureIt, figureEnd;
                    for (figureIt = figureList.begin(), figureEnd = figureList.end(); figureIt != figureEnd; ++figureIt) {
                        Figure& figure = **figureIt;
                        figure.render(vbo, context);
                    }
                }
            }
        }
    }
}