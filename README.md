# GLS — GL Stuff Library

© binary1248, GLS is provided under the *Mozilla Public License Version 2.0*
(see `LICENSE` for details)

## Requirements

GLS is a header-only library. It does not depend on anything other than the
standard C++ libraries. 

GLS requires a compiler with C\+\+11 support. Not all features are used,
however many newer features are used that might not be supported by all
compilers even though they claim C\+\+11 support. Your mileage may vary.

In order to build the examples, you will need SFML 2.3 or the *gl_dev* branch
of SFML 2.1. Previous versions do not provide a way of explicitly creating an
OpenGL 3.2 core profile context, so minor alterations to the example code have
to be made if using an older version of SFML. SFML is available from
http://sfml-dev.org/ or [GitHub](https://github.com/SFML/SFML).

The build system in use is [CMake](http://cmake.org/). It's available for all
major platforms.

## Installing

Since GLS is a header-only library, just copy the GLS directory to a location
of your choosing and add the include directory to your compiler's search paths.
Alternatively, you can generate an installation script through the CMake
configuration.

To generate a configuration for the examples and installation, run

```bash
cmake .
```

inside the root directory of GLS. Alternatively just generate a configuration
using the CMake GUI. The required entries should be set, and if not, they are
well documented so they can be easily filled manually.

After generating the configuration, build the install target.

## Using GLS in your own projects

To use GLS, include the necessary headers. The GLS meta-header:

```cpp
#include <gls.hpp>
```

**IMPORTANT:** Make sure to include GLS headers *after* you include your OpenGL
headers. GLS makes use of OpenGL definitions and entry points and will cause
your code to fail to compile if it can't find them.

Since GLS is a header-only library, building it is not required before using
it. It follows that there are no library files to link as well. Simply make
sure that your compiler is able to find GLS's include directory.

If you are using CMake for your project and you've properly installed GLS (see
*Installing*), you can automate the process of finding GLS. First, copy the
file `cmake/Modules/FindGLS.cmake` to your project's CMake Modules directory
and make sure CMake knows that it should search in this directory. Then add the
following to your project's `CMakeLists.txt` file:

```cmake
find_package( GLS REQUIRED )
include_directories( ${GLS_INCLUDE_DIR} )
```

In case CMake can't find your GLS installation, you can still specify the path
manually by setting the `GLS_ROOT` CMake variable.

Because GLS is a helper library for OpenGL, be sure to link the OpenGL library
for your platform to your final application as well.

## Documentation

A pre-generated version of the documentation can be found online at:
http://binary1248.github.io/GLS

You can also generate the API documentation yourself. Enable the
`GLS_BUILD_DOC` entry in CMake, configure and generate the build configuration.
Run the doc target to generate the API documentation from the headers. You will
need [Doxygen](http://www.stack.nl/~dimitri/doxygen/) to do this.

## Contact

The `AUTHORS` file lists contributors with contact information.
