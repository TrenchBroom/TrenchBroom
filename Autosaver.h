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

#import <Foundation/Foundation.h>

@class MapDocument;

@interface Autosaver : NSObject {
    MapDocument* map;
    NSTimeInterval saveInterval;
    NSTimeInterval idleInterval;
    int numberOfBackups;
    
    NSDate* lastAction;
    NSDate* lastSave;
    NSTimer* timer;
}

- (id)initWithMap:(MapDocument *)theMap saveInterval:(NSTimeInterval)theSaveInterval idleInterval:(NSTimeInterval)theIdleInterval numberOfBackups:(int)theNumberOfBackups;

- (void)updateLastAction;

@end
