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

#ifndef TrenchBroom_DragTargetTool_h
#define TrenchBroom_DragTargetTool_h

namespace TrenchBroom {
    namespace Controller {
        class ToolEvent;
        
        class DragInfo {
        public:
            ToolEvent& event;
            std::string name;
            void* package;

            DragInfo(ToolEvent& event) : event(event) {}
        };
        
        class DragTargetTool {
        public:
            DragTargetTool() {}
            virtual ~DragTargetTool() {}
            
            virtual void activate(const DragInfo& info) = 0;
            virtual void deactivate(const DragInfo& info) = 0;
            virtual void move(const DragInfo& info) = 0;
            virtual bool drop(const DragInfo& info) = 0;
        };
    }
}

#endif
