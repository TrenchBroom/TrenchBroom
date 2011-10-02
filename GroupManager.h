/*
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

static NSString* const GroupsChanged = @"GroupsChanged";


@class MapDocument;
@protocol Entity;
@protocol Brush;

@interface GroupManager : NSObject {
    MapDocument* map;
    NSMutableArray* groups;
    int visibleGroupCount;
}

- (id)initWithMap:(MapDocument *)theMap;

- (NSArray *)groups;
- (void)setGroup:(id <Entity>)theGroup name:(NSString *)theName;
- (void)setGroup:(id <Entity>)theGroup visibility:(BOOL)theVisibility;
- (BOOL)isVisible:(id <Entity>)theGroup;
- (BOOL)allGroupsInvisible;

@end
