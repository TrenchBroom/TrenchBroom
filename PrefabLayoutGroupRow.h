/*
Copyright (C) 2010-2011 Kristian Duske

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

@protocol PrefabGroup;
@protocol Prefab;
@class GLFontManager;

@interface PrefabLayoutGroupRow : NSObject {
    id <PrefabGroup> prefabGroup;
    NSRect titleBarBounds;
    NSRect titleBounds;
    NSRect bounds;
    NSMutableArray* cells;
}

- (id)initWithPrefabGroup:(id <PrefabGroup>)thePrefabGroup prefabsPerRow:(int)prefabsPerRow atPos:(NSPoint)thePos width:(float)theWidth innerMargin:(float)innerMargin fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont;

- (id <PrefabGroup>)prefabGroup;
- (NSArray *)cells;

- (id <Prefab>)prefabAt:(NSPoint)pos;

- (NSRect)titleBarBounds;
- (NSRect)titleBounds;
- (NSRect)bounds;

@end
