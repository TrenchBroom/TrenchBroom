/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "Preferences.h"

namespace TrenchBroom {
    namespace Preferences {
        Preference<Color> BackgroundColor = Preference<Color>("Colors/Background", Color(0.0f, 0.0f, 0.0f, 1.0f));
        Preference<Color> XAxisColor = Preference<Color>("Colors/Background", Color(1.0f, 0.0f, 0.0f, 1.0f));
        Preference<Color> YAxisColor = Preference<Color>("Colors/Background", Color(0.0f, 1.0f, 0.0f, 1.0f));
        Preference<Color> ZAxisColor = Preference<Color>("Colors/Background", Color(0.0f, 0.0f, 1.0f, 1.0f));
        Preference<Color> FaceColor = Preference<Color>("Colors/Faces", Color(0.2f,  0.2f,  0.2f,  1.0f));
        Preference<Color> EdgeColor = Preference<Color>("Colors/Edges", Color(.7f,  0.7f,  0.7f,  1.0f));

        Preference<float> CameraLookSpeed = Preference<float>("Controls/Camera/Look speed", 0.5f);
        Preference<bool>  CameraLookInvertH = Preference<bool>("Controls/Camera/Invert horizontal look", false);
        Preference<bool>  CameraLookInvertV = Preference<bool>("Controls/Camera/Invert vertical look", false);
        Preference<float> CameraPanSpeed = Preference<float>("Controls/Camera/Pan speed", 0.5f);
        Preference<bool>  CameraPanInvertH = Preference<bool>("Controls/Camera/Invert horizontal pan", false);
        Preference<bool>  CameraPanInvertV = Preference<bool>("Controls/Camera/Invert vertical pan", false);
        Preference<float> CameraMoveSpeed = Preference<float>("Controls/Camera/Move speed", 0.3f);
        Preference<bool> CameraEnableAltMove = Preference<bool>("Controls/Camera/Use alt to move", false);
        Preference<bool> CameraMoveInCursorDir = Preference<bool>("Controls/Camera/Move camera in cursor dir", false);
    }
}
