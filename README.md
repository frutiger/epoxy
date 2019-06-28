# epoxy

A suite of idiomatic, cross-platform, zero-cost C++17 libraries to bind to
embedder APIs for modern dynamically typed runtimes.  The suite also includes
`epoxy`, a library of helper facilities that ease the burden of writing
converters, adapters and marshallers for using these runtimes.

Each library has a corresponding program indicating idiomatic usage in the
`examples` directory that reads a program from standard input and prints the
result.

## Supported Runtimes

This table lists the supported runtimes, versions and their library names.

| Runtime | Version(s)   | Zero-cost Library |
|---------|--------------|-------------------|
| V8      | 7.5 or newer | `crude`           |

## Build

This project builds using `CMake`.  Any platform is supported as long as the
underlying runtime supports that platform.  See the third party documentation
for any specific runtime for such further details.

Some runtimes may require environment variables to locate their libraries:

### crude

The following environment variable must be set:

  * `$V8_DIR`, such that `$V8_DIR/include` should contain the public V8 headers

The following environment variable may be set:

  * `$V8_LIB`, such that `$V8_DIR/$V8_LIB` contains the V8 libraries; this
  defaults to `lib` if not specified.  This is useful for specifying e.g.
  `out/release.x64` for local builds of V8.

`crude` assumes no external startup data is required for V8 to run.

```
$ export V8_DIR=/path/to/v8 V8_LIB=relative/path/to/libs
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```

