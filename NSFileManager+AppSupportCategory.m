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

#import "NSFileManager+AppSupportCategory.h"


@implementation NSFileManager (AppSupportCategory)

- (NSString *)findSystemFolderType:(int)folderType forDomain:(int)domain
{
    FSRef folder;
    OSErr err = noErr;
    CFURLRef url;
    NSString *result = nil;
    
    err = FSFindFolder(domain, folderType, false, &folder);
    if (err == noErr) {
        url = CFURLCreateFromFSRef(kCFAllocatorDefault, &folder);
        result = [(NSURL *)url path];
        CFRelease(url);
    }
    
    return result;
}

- (NSString *)findApplicationSupportFolder {
    NSString* applicationSupportFolder = [self findSystemFolderType:kApplicationSupportFolderType forDomain:kUserDomain];
    NSString* bundlePath = [[NSBundle mainBundle] bundlePath];
    NSString* appName = [[self displayNameAtPath:bundlePath] stringByDeletingPathExtension];
    
    return [NSString pathWithComponents:[NSArray arrayWithObjects:applicationSupportFolder, appName, nil]];
}

@end