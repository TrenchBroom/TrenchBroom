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

#import "IntData.h"


@implementation IntData
- (id)init {
    if ((self = [super init])) {
        data = [[NSMutableData alloc] init];
        count = 0;
    }
    
    return self;
}

- (id)initDataWithCapacity:(int)capacity {
    if ((self = [super init])) {
        data = [[NSMutableData alloc] initWithLength:capacity * sizeof(int)];
        count = 0;
    }
    
    return self;
}

- (void)appendInt:(int)value {
    [data appendBytes:(char *)&value length:sizeof(int)];
    count++;
}

- (const void*)bytes{
    return [data bytes];
}

- (int)count {
    return count;
}

- (void)dealloc {
    [data release];
    [super dealloc];
}
@end
