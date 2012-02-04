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

#import <Cocoa/Cocoa.h>

@class CompassView;
@class MapWindowController;

@interface QuickBarWindowController : NSWindowController <NSWindowDelegate> {
    IBOutlet NSTextField* selectionOriginXField;
    IBOutlet NSTextField* selectionOriginYField;
    IBOutlet NSTextField* selectionOriginZField;
    IBOutlet NSTextField* selectionSizeXField;
    IBOutlet NSTextField* selectionSizeYField;
    IBOutlet NSTextField* selectionSizeZField;
    IBOutlet NSTextField* cameraPosXField;
    IBOutlet NSTextField* cameraPosYField;
    IBOutlet NSTextField* cameraPosZField;
    IBOutlet NSTextField* cameraDirXField;
    IBOutlet NSTextField* cameraDirYField;
    IBOutlet NSTextField* cameraDirZField;
    IBOutlet CompassView* compassView;
    IBOutlet NSTextField* entityClassnameField;
    IBOutlet NSTextField* entitySpawnFlagsField;
    IBOutlet NSTextField* faceXOffsetField;
    IBOutlet NSTextField* faceYOffsetField;
    IBOutlet NSTextField* faceXScaleField;
    IBOutlet NSTextField* faceYScaleField;
    IBOutlet NSTextField* faceRotationField;
    IBOutlet NSTextField* faceTextureField;
    MapWindowController* mapWindowController;
}

- (void)setMapWindowController:(MapWindowController *)theMapWindowController;

- (IBAction)switchToXYView:(id)sender;
- (IBAction)switchToXZView:(id)sender;
- (IBAction)switchToYZView:(id)sender;

- (IBAction)selectAll:(id)sender;
- (IBAction)selectNone:(id)sender;
- (IBAction)selectEntity:(id)sender;

@end
