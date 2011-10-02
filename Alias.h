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
#import "Math.h"

@class AliasFrame;
@class AliasSkin;

typedef struct {
    BOOL onseam;
    int s;
    int t;
} TSkinVertex;

typedef struct {
    BOOL front;
    int vertices[3];
} TSkinTriangle;

typedef struct {
    TVector3f position;
    TVector2f texCoords;
    TVector3f norm;
} TFrameVertex;

typedef struct {
    int x,y,z,i;
} TPackedFrameVertex;

typedef struct {
    TFrameVertex vertices[3];
} TFrameTriangle;

@interface Alias : NSObject {
    NSString* name;
    NSMutableArray* frames;
    NSMutableArray* skins;
}

- (id)initWithName:(NSString *)theName data:(NSData *)theData;

- (NSString *)name;
- (AliasFrame *)firstFrame;

- (AliasSkin *)firstSkin;
- (AliasSkin *)skinWithIndex:(int)theSkinIndex;

@end
