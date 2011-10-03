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

#import "WadEntry.h"

@interface WadTextureEntry : WadEntry {
    @private
    int width;
    int height;
    NSData* mip0;
    NSData* mip1;
    NSData* mip2;
    NSData* mip3;
}
- (id)initWithName:(NSString *)theName width:(int)theWidth height:(int)theHeight mip0:(NSData *)mip0Data mip1:(NSData *)mip1Data mip2:(NSData *)mip2Data mip3:(NSData *)mip3Data;

- (int)width;
- (int)height;

- (NSData *)mip0;
- (NSData *)mip1;
- (NSData *)mip2;
- (NSData *)mip3;

@end
