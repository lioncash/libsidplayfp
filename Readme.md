# libsidplayfp

libsidplayfp, but with a CMake buildsystem.

### Why?

Autotools is (IMO) not a convenient choice for a cross-platform buildsystem in the
context of non-UNIX platforms. e.g. Windows users shouldn't need to install msys or cygwin in
order to build a simple library and applications that interface with it. It should be sufficient
to install MSVC and perform a build out of the box.

CMake makes it nicer to consume these libraries on all platforms and also makes it
relatively painless to incorporate third-party tooling (dependency analysis, static analysis, ubsan, etc).
