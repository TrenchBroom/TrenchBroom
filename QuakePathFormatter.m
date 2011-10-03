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

#import "QuakePathFormatter.h"

@implementation QuakePathFormatter

- (NSString *)stringForObjectValue:(id)obj {
    if ([obj isKindOfClass:[NSString class]])
        return [NSString stringWithString:obj];
    return nil;
}

- (BOOL)getObjectValue:(id *)obj forString:(NSString *)string errorDescription:(NSString **)error {
    NSFileManager* fileManager = [NSFileManager defaultManager];
    BOOL directory;
    BOOL exists = [fileManager fileExistsAtPath:string isDirectory:&directory];
    if (!exists) {
        if (error)
            *error = [NSString stringWithFormat:@"%@ does not exist", string];
        return NO;
    }
    
    if (!directory) {
        if (error)
            *error = [NSString stringWithFormat:@"%@ is not a directory", string];
        return NO;
    }
    
    *obj = [NSString stringWithString:string];
    return YES;
}

@end
