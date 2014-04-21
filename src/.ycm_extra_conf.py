# This file is NOT licensed under the GPLv3, which is the license for the rest
# of YouCompleteMe.
#
# Here's the license text for this file:
#
# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# For more information, please refer to <http://unlicense.org/>

import os
import ycm_core

# These are the compilation flags that will be used in case there's no
# compilation database set (by default, one is not set).
# CHANGE THIS LIST OF FLAGS. YES, THIS IS THE DROID YOU HAVE BEEN LOOKING FOR.
flags = [
'-x c++',
'-arch i386',
'-fmessage-length=0',
'-fdiagnostics-show-note-include-stack',
'-fmacro-backtrace-limit=0',
'-Wno-trigraphs',
'-fpascal-strings',
'-O0',
'-Wno-missing-field-initializers',
'-Wno-missing-prototypes',
'-Wno-return-type',
'-Wno-non-virtual-dtor',
'-Wno-overloaded-virtual',
'-Wno-exit-time-destructors',
'-Wno-missing-braces',
'-Wparentheses',
'-Wswitch',
'-Wno-unused-function',
'-Wno-unused-label',
'-Wno-unused-parameter',
'-Wno-unused-variable',
'-Wunused-value',
'-Wno-empty-body',
'-Wno-uninitialized',
'-Wno-unknown-pragmas',
'-Wno-shadow',
'-Wno-four-char-constants',
'-Wno-conversion',
'-Wno-constant-conversion',
'-Wno-int-conversion',
'-Wno-bool-conversion',
'-Wno-enum-conversion',
'-Wno-shorten-64-to-32',
'-Wno-newline-eof',
'-Wno-c++11-extensions',
'-DCMAKE_INTDIR=\"Debug\"',
'-DGLEW_STATIC',
'-D_FILE_OFFSET_BITS=64',
'-DWXUSINGDLL',
'-D__WXMAC__',
'-D__WXOSX__',
'-D__WXOSX_COCOA__',
'-isysroot /Developer/SDKs/MacOSX10.6.sdk',
'-fasm-blocks',
'-fstrict-aliasing',
'-Wdeprecated-declarations',
'-Winvalid-offsetof',
'-mmacosx-version-min=10.6',
'-Wno-sign-conversion',
'-I/Users/kristian/Documents/Code/TrenchBroom/Xcode/Debug/include',
'-I/Developer/SDKs/MacOSX10.6.sdk/usr/local/lib/wx/include/osx_cocoa-unicode-3.0',
'-I/Developer/SDKs/MacOSX10.6.sdk/usr/local/include/wx-3.0',
'-I/Users/kristian/Documents/Code/TrenchBroom/lib/include',
'-I/Users/kristian/Documents/Code/TrenchBroom/src',
'-I/Users/kristian/Documents/Code/TrenchBroom/test/src',
'-I/Users/kristian/Documents/Code/TrenchBroom/Xcode/TrenchBroom.build/Debug/TrenchBroom.build/DerivedSources/i386',
'-I/Users/kristian/Documents/Code/TrenchBroom/Xcode/TrenchBroom.build/Debug/TrenchBroom.build/DerivedSources',
'-Wmost',
'-Wno-four-char-constants',
'-Wno-unknown-pragmas',
'-F/Users/kristian/Documents/Code/TrenchBroom/Xcode/Debug',
'-I/usr/local/include/wx-3.0',
'-Wall',
'-pedantic',
'-Wno-format',
'-std=c++0x',
'-Qunused-arguments',
'-fcolor-diagnostics',
'-fno-inline',
'-g3',
'-MMD']


# Set this to the absolute path to the folder (NOT the file!) containing the
# compile_commands.json file to use that instead of 'flags'. See here for
# more details: http://clang.llvm.org/docs/JSONCompilationDatabase.html
#
# Most projects will NOT need to set this to anything; you can just change the
# 'flags' list of compilation flags. Notice that YCM itself uses that approach.
compilation_database_folder = ''

if os.path.exists( compilation_database_folder ):
  database = ycm_core.CompilationDatabase( compilation_database_folder )
else:
  database = None

SOURCE_EXTENSIONS = [ '.cpp', '.cxx', '.cc', '.c', '.m', '.mm' ]

def DirectoryOfThisScript():
  return os.path.dirname( os.path.abspath( __file__ ) )


def MakeRelativePathsInFlagsAbsolute( flags, working_directory ):
  if not working_directory:
    return list( flags )
  new_flags = []
  make_next_absolute = False
  path_flags = [ '-isystem', '-I', '-iquote', '--sysroot=' ]
  for flag in flags:
    new_flag = flag

    if make_next_absolute:
      make_next_absolute = False
      if not flag.startswith( '/' ):
        new_flag = os.path.join( working_directory, flag )

    for path_flag in path_flags:
      if flag == path_flag:
        make_next_absolute = True
        break

      if flag.startswith( path_flag ):
        path = flag[ len( path_flag ): ]
        new_flag = path_flag + os.path.join( working_directory, path )
        break

    if new_flag:
      new_flags.append( new_flag )
  return new_flags


def IsHeaderFile( filename ):
  extension = os.path.splitext( filename )[ 1 ]
  return extension in [ '.h', '.hxx', '.hpp', '.hh' ]


def GetCompilationInfoForFile( filename ):
  # The compilation_commands.json file generated by CMake does not have entries
  # for header files. So we do our best by asking the db for flags for a
  # corresponding source file, if any. If one exists, the flags for that file
  # should be good enough.
  if IsHeaderFile( filename ):
    basename = os.path.splitext( filename )[ 0 ]
    for extension in SOURCE_EXTENSIONS:
      replacement_file = basename + extension
      if os.path.exists( replacement_file ):
        compilation_info = database.GetCompilationInfoForFile(
          replacement_file )
        if compilation_info.compiler_flags_:
          return compilation_info
    return None
  return database.GetCompilationInfoForFile( filename )


def FlagsForFile( filename, **kwargs ):
  if database:
    # Bear in mind that compilation_info.compiler_flags_ does NOT return a
    # python list, but a "list-like" StringVec object
    compilation_info = GetCompilationInfoForFile( filename )
    if not compilation_info:
      return None

    final_flags = MakeRelativePathsInFlagsAbsolute(
      compilation_info.compiler_flags_,
      compilation_info.compiler_working_dir_ )

    # NOTE: This is just for YouCompleteMe; it's highly likely that your project
    # does NOT need to remove the stdlib flag. DO NOT USE THIS IN YOUR
    # ycm_extra_conf IF YOU'RE NOT 100% SURE YOU NEED IT.
    try:
      final_flags.remove( '-stdlib=libc++' )
    except ValueError:
      pass
  else:
    relative_to = DirectoryOfThisScript()
    final_flags = MakeRelativePathsInFlagsAbsolute( flags, relative_to )

  return {
    'flags': final_flags,
    'do_cache': True
  }
