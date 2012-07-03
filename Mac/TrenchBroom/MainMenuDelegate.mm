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

#import "MainMenuDelegate.h"

#import "EditorHolder.h"
#import "MapDocument.h"

#import "Controller/Editor.h"
#import "Model/Map/Map.h"
#import "Model/Selection.h"

using namespace TrenchBroom;

@implementation MainMenuDelegate

- (void)menuNeedsUpdate:(NSMenu *)menu {
    NSApplication* app = [NSApplication sharedApplication];
    NSWindow* keyWindow = [app keyWindow];
    NSWindowController* controller = [keyWindow windowController];
    NSDocument* document = [controller document];
    
    if ([actionsMenuItem hasSubmenu])
        [actionsMenuItem setSubmenu:nil];
    [actionsMenuItem setEnabled:NO];
    
    if ([document isKindOfClass:[MapDocument class]]) {
        MapDocument* mapDocument = (MapDocument*)document;
        EditorHolder* editorHolder = [mapDocument editorHolder];
        Controller::Editor* editor = (Controller::Editor*)[editorHolder editor];
        Model::Map& map = editor->map();
        Model::Selection& selection = map.selection();
        
        switch (selection.mode()) {
            case Model::TB_SM_FACES:
                [actionsMenuItem setSubmenu:textureActionsMenu];
                [actionsMenuItem setEnabled:YES];
                break;
            case Model::TB_SM_BRUSHES:
            case Model::TB_SM_ENTITIES:
            case Model::TB_SM_BRUSHES_ENTITIES:
                [actionsMenuItem setSubmenu:objectActionsMenu];
                [actionsMenuItem setEnabled:YES];
                break;
            default:
                break;
        }
    }
}

@end
