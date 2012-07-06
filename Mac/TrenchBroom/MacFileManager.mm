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

#import "MacFileManager.h"

namespace TrenchBroom {
    namespace IO {
        bool MacFileManager::isDirectory(const std::string& path) {
            NSFileManager* fileManager = [NSFileManager defaultManager];
            
            BOOL directory = false;
            BOOL exists = [fileManager fileExistsAtPath:[NSString stringWithCString:path.c_str() encoding:NSASCIIStringEncoding] isDirectory:&directory];
            
            return exists && directory;
        }
        
        bool MacFileManager::exists(const std::string& path) {
            NSFileManager* fileManager = [NSFileManager defaultManager];
            return [fileManager fileExistsAtPath:[NSString stringWithCString:path.c_str() encoding:NSASCIIStringEncoding]];
        }
        
        std::vector<std::string> MacFileManager::directoryContents(const std::string& path, std::string extension) {
            std::vector<std::string> result;
            
            NSFileManager* fileManager = [NSFileManager defaultManager];
            NSArray* entries = [fileManager contentsOfDirectoryAtPath:[NSString stringWithCString:path.c_str() encoding:NSASCIIStringEncoding] error:NULL];
            
            if (entries != nil) {
                for (NSString* entry in entries) {
                    std::string entryName = [entry cStringUsingEncoding:NSASCIIStringEncoding];
                    if (extension.empty() || pathExtension(entryName) == extension)
                        result.push_back(pathComponents(entryName).back());
                }
            }
            
            return result;
        }
    }
}