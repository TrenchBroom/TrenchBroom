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

#import "NSString+StdStringAdditions.h"

@implementation NSString (StdStringAdditions)

NSStringEncoding WStrEncoding =
#if TARGET_RT_BIG_ENDIAN
NSUTF32BigEndianStringEncoding;
#else
NSUTF32LittleEndianStringEncoding;
#endif

+ (NSString*)stringWithStdString:(const std::string&)string {
    return [self stringWithCString:string.c_str() encoding:NSASCIIStringEncoding];
}

+ (NSString*)stringWithStdWString:(const std::wstring&)string {
    char* data = (char*)string.data();
    size_t size = string.size() * sizeof(wchar_t);
    
    return [[[NSString alloc] initWithBytes:data length:size encoding:WStrEncoding] autorelease];
}

- (std::string) stdString {
    return [self cStringUsingEncoding:NSASCIIStringEncoding];
}

- (std::wstring) stdWString {
    NSData* asData = [self dataUsingEncoding:WStrEncoding];
    return std::wstring((wchar_t*)[asData bytes], [asData length] / sizeof(wchar_t));
}

@end
