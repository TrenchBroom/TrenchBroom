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

#import <Cocoa/Cocoa.h>
#import "Prefab.h"
#import "Map.h"

@class MutablePrefabGroup;
@class MapDocument;

@interface MutablePrefab : NSObject <Prefab, Map> {
    @private
    MutablePrefabGroup* prefabGroup;
    NSString* name;
    NSNumber* prefabId;
    BOOL readOnly;
    NSMutableArray* entities;
    TBoundingBox bounds;
    TBoundingBox maxBounds;
    TBoundingBox* worldBounds;
    TVector3f center;
    BOOL valid;
}

- (id)initWithWorldBounds:(TBoundingBox *)theWorldBounds name:(NSString *)theName group:(MutablePrefabGroup *)thePrefabGroup readOnly:(BOOL)isReadOnly;

- (void)setName:(NSString *)theName;
- (void)setPrefabGroup:(MutablePrefabGroup *)thePrefabGroup;
@end
