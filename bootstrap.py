#!/usr/bin/env python

import ninja_syntax
import os, sys
import fnmatch
from distutils.spawn import find_executable
import itertools
import argparse

install_dir =  'include' if sys.platform == 'win32' else os.path.join('/usr', 'include')

# command line stuff
parser = argparse.ArgumentParser(usage='%(prog)s [options...]')
parser.add_argument('--debug', action='store_true', help='compile with debug flags')
parser.add_argument('--cxx', metavar='<compiler>', help='compiler name to use (default: g++)', default='g++')
parser.add_argument('--install-dir', metavar='<dir>', help='directory to install the headers to', default=install_dir)
parser.add_argument('--quiet', '-q', action='store_true', help='suppress warning output')
parser.epilog = """In order to install gears, administrative privileges might be required.
Note that installation is done through the 'ninja install' command. To uninstall, the
command used is 'ninja uninstall'. The default installation directory for this
system is {}""".format(install_dir)
args = parser.parse_args()

script_directory = os.path.dirname(os.path.realpath(sys.argv[0]))
gears_path = os.path.join(script_directory, 'gears')
os.chdir(script_directory)

def warning(string):
    if not args.quiet:
        print('warning: {}'.format(string))

# configuration
doxygen_path = find_executable('doxygen')
if doxygen_path == None:
    warning('doxygen executable not found')

if find_executable(args.cxx) == None:
    parser.error('compiler {} not found'.format(args.cxx))

if 'g++' not in args.cxx and 'clang++' not in args.cxx:
    warning('compiler not explicitly supported: {}'.format(args.cxx))

# general variables
include = [ '.', 'gears' ]
depends = ['tests']
cxxflags = [ '-Wall', '-Wextra', '-pedantic', '-std=c++11' ]
ignored_warnings = ['mismatched-tags', 'switch']
copy_command = 'robocopy /COPYALL /E {dir} $in' if sys.platform == 'win32' else 'cp -rf {dir} $in'
remove_command = 'rmdir /S /Q {dir}' if sys.platform == 'win32' else 'rm -rf {dir}'
copy_command = copy_command.format(dir=gears_path)
remove_command = remove_command.format(dir=os.path.join(args.install_dir, 'gears'))

if args.debug:
    cxxflags.extend(['-g', '-O0', '-DDEBUG'])
else:
    cxxflags.extend(['-DNDEBUG', '-O3'])

if args.cxx == 'clang++':
    ignored_warnings.extend(['constexpr-not-const', 'unused-value'])

builddir = 'bin'
objdir = 'obj'
tests = os.path.join(builddir, 'tests')

# utilities
def flags(*args):
    return ' '.join(itertools.chain(*args))

def includes(l):
    return ['-I"{}"'.format(x) for x in l]

def ignored(l):
    return ['-Wno-{}'.format(x) for x in l]

def dependencies(l):
    return ['-isystem "{}"'.format(x) for x in l]

def get_files(directory, pattern):
    for root, directories, files in os.walk(directory):
        for f in files:
            if fnmatch.fnmatch(f, pattern):
                yield os.path.join(root, f)

def object_file(f):
    (root, ext) = os.path.splitext(f)
    return os.path.join(objdir, root + '.o')

# ninja file
ninja = ninja_syntax.Writer(open('build.ninja', 'w'))

# variables
ninja.variable('ninja_required_version', '1.3')
ninja.variable('builddir', 'bin')
ninja.variable('cxx', args.cxx)
ninja.variable('cxxflags', flags(cxxflags + includes(include) + dependencies(depends) + ignored(ignored_warnings)))

# rules
ninja.rule('bootstrap', command = ' '.join(['python'] + sys.argv), generator = True)
ninja.rule('compile', command = '$cxx -MMD -MF $out.d -c $cxxflags $in -o $out',
                      deps = 'gcc', depfile = '$out.d',
                      description = 'Compiling $in to $out')
ninja.rule('link', command = '$cxx $cxxflags $in -o $out', description = 'Creating $out')
ninja.rule('runner', command = tests)

if doxygen_path:
    ninja.rule('documentation', command = '{} $in'.format(doxygen_path), description = 'Generating documentation')

ninja.rule('installer', command = copy_command)
ninja.rule('uninstaller', command = remove_command)

# builds
ninja.build('build.ninja', 'bootstrap', implicit = sys.argv[0])

source_files = list(get_files('tests', '*.cpp'))
object_files = [object_file(f) for f in source_files]
for f in source_files:
    ninja.build(object_file(f), 'compile', inputs = f)

ninja.build(tests, 'link', inputs = object_files)
ninja.build('tests', 'phony', inputs = tests)
ninja.build('install', 'installer', inputs = args.install_dir)
ninja.build('uninstall', 'uninstaller')
ninja.build('run', 'runner', implicit = 'tests')

if doxygen_path:
    ninja.build('doxygen', 'documentation', inputs = os.path.join(script_directory, 'Doxyfile'))
    ninja.build('docs', 'phony', 'doxygen')

ninja.default('run')
