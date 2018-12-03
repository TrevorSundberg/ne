# ne - Native Everywhere

**ne** is a standard specification, rather than a specific implementation. The standard includes:

1. Concise cross platform API for modern operating systems. Includes [Vulkan](https://www.khronos.org/vulkan/) for graphics.
2. Portable executable `.ne` that can be interpreted on every operating system.
3. Permission based secure sandbox based on native client research.
4. Package management for sharing libraries.

*This repository contains the standard documents as well as example implementations and releases.*

## Your First Application

The `.ne` executable is a `.zip` archive that contains `.ll` LLVM text files, an optional `manifest.yaml` file, and any other files you wish to package with your executable.

For example, copy this code into a new a `main.ll` file:
```llvm
declare void @ne_sandbox_hello_world()

define i32 @main()
{
  call void @ne_sandbox_hello_world()
  ret i32 0
}
```

The equivalent C program would be:
```C
#include "ne.h" /* extern void ne_sandbox_hello_world(void); */

int main(void)
{
  ne_sandbox_hello_world();
  return 0;
}
```

Zip up the `main.ll` into `main.zip`, then rename `main.zip` file to `main.ne`. Congratulations! You have created your first ne-executable. The call to `ne_sandbox_hello_world` will print or show a *hello world* message. This function exists for newcomers and testing purposes. Note that our `.ne` executable could also be used as a library if we were to use `external` linkage for our own functions. An `.ne` file can include other `.ne` files, but note that this is considered static linking, and we prefer users to dynamically link libraries using the package manager and `manifest.yaml` files instead which is more compatible with licenses such as the LGPL.

When `main.ne` is run, the contained LLVM text files are automatically compiled into native assembly and linked against that platform's ne-API. Any function with C-linkage that starts with `ne_` may be *optionally linked* against. If a platform does not support a given `ne_` API call, then an empty function will be generated that returns the equivalent of 0, null, false, etc. This allows platforms to implement parts of the ne-API, and even extend the API without worrying about linkage problems (much like OpenGL's extensions). This is why the ne-API uses `supported` calls, such as `ne_bool ne_socket_supported(void)`. Even if `ne_socket_supported` is not implemented, it will return false due to *optional linkage*. This can be used for hosts to offer extended functionality, for example if an application runs under [Steam](https://store.steampowered.com/) it could expose the Steam API by introducing an `ne_steam_supported` function and corresponding `ne_steam_...` calls.

To build a more complex application or library for **ne** with your favorite native languages (C / C++ / Rust / etc.), download the [latest release](releases) for your operating system. Instructions can be found [here](https://github.com/TrevorSundberg/ne/wiki) for setting up compiler tool-chains to support outputting `.ne` executables directly.

## Compile Once

Compilers should target 64 bit pointers with little endian format.

To support the idea that we compile once only and run on every platform, it means that compile time operations such as the `sizeof` operator in C will only be evaluated once for all platforms. Because of this, it means that there are requirements on alignment, pointer sizes, padding, endianness, etc. All compilations should result in the same `sizeof` for structures and primitives.

The following LLVM data layout is what all sandboxes use:

```
e - little endian
p:64:64:64 - 64-bit pointers with 64-bit alignment.
p[n]:64:64:64 - Other address spaces are assumed to be the same as the default address space.
S0 - natural stack alignment is unspecified
i1:8:8 - i1 is 8-bit (byte) aligned
i8:8:8 - i8 is 8-bit (byte) aligned
i16:16:16 - i16 is 16-bit aligned
i32:32:32 - i32 is 32-bit aligned
i64:32:64 - i64 has ABI alignment of 32-bits but preferred alignment of 64-bits
f16:16:16 - half is 16-bit aligned
f32:32:32 - float is 32-bit aligned
f64:64:64 - double is 64-bit aligned
f128:128:128 - quad is 128-bit aligned
v64:64:64 - 64-bit vector is 64-bit aligned
v128:128:128 - 128-bit vector is 128-bit aligned
a:0:64 - aggregates are 64-bit aligned
```

## Versioning

A proper `.ne` package uses the [SemVer](http://semver.org/) schema for versioning. For example, if we were to distribute our `main.ne` as a package, we should rename it to `main_1.0.0.ne`. Note that the underscore before the version is required. A proper named library or executable follows the semantic `name_major.minor.patch.ne`. Obviously the name `main` is not a great library name, so pick something unique!

## Manifest YAML File

The `manifest.yaml` file describes package dependencies, required permissions for your application to run, and ways that the operating system may interact with your application (for example if your application handles any file extensions).

An example manifest file:
```yaml
- dependency:
    name: ne_core
    major: 1
    minor: 0

# libc automatically pulls in dependencies on ne_core, ne_memory, ne_filesystem, etc.
- dependency:
    name: libc
    major: 1

- dependency:
    name: curl
    major: 7
    minor: 60
    patch: 0
```

#### Package Dependency

```yaml
- dependency:
    name: ne_socket
    major: 1
    minor: 1
    patch: 2
    require: request_permission
```
The `minor` and `patch` are optional. If the patch is left out, the latest patch will be retrieved, and same for the minor. The major is required because it represents breaking API changes. A dependency will be downloaded by the sandbox before your application is run, and in the event that a package cannot be retrieved the sandbox will emit an error.

The `require` field is optional and defaults to `request_permission`. Valid values are:
 - `request_permission` - The sandbox must support and grant permission to use prior to your program's execution. Calls to `ne_socket_supported` and `ne_socket_request_permission` will return true.
 - `supported` - The sandbox must support the library, but you do not have implicit permission to use it. Calls to `ne_socket_supported` will return true.
 - `none` - The sandbox may not support the library and you do not have implicit permission to use it. Calls to `ne_socket_supported` may return false. This is useful when a program can run with or without `ne_socket`, but still wants to download the package and compile against the headers.

#### Sandbox Pragma

```yaml
-pragma:
    name: anything
    other-data: 1234
```

A pragma is sandbox dependent and is used to communicate information to a particular sandbox. The values have no meaning otherwise. The use of pragmas is discouraged as we hope that `.ne` files can remain agnostic of sandbox, however we understand the need for these pragmas on occasion.

## Packages

A package consists of the `.ne` file as well as header files for C / C++ or any other auxiliary files. It is up to the sandbox application to decide how and where packages are securely downloaded from. We highly recommend that sandboxes piggyback on the Node.js package management system [npm](https://www.npmjs.com/).

For example using `npm` you can install the [Lua package](https://www.lua.org/about.html):

```
npm install ne_lua
```

A side goal of **ne** is to make programming in C and C++ as fun and easy as it is using frameworks like [Node.js](https://nodejs.org/). To do this, we recommend libraries always prefer relative include paths. This means not forcing a user to setup custom include directories. For example, if you write a `main.c` file, and then install a package like `ne_lua`, it will create a directory called `ne_lua` with the `.ne` library and all the Lua headers. Lua headers only include each other using relative includes (for example `lapi.h` in Lua includes `"lstate.h"`), which means you can avoid setting up custom include paths and just write include `"ne_lua/lua.h"`.

Unfortunately not all C libraries work this way. For example [FreeType](https://www.freetype.org/index.html) includes its own files using angle brackets such as including `<ft2build.h>`, and also does not include files within the same directory using relative paths. This means that the user is forced to setup custom include paths. That said, FreeType is still a great library!

**We encourage those uploading packages to revise the includes to be relative and as flat as possible.**

When you compile your application, it will output to a `.ll` file and no linking phase will occur so you never need to worry about `.lib` or `.o` files. Simply place a [dependency](#package-dependency) element in the `manifest.yaml` file, for example:

```yaml
- dependency:
    name: lua
    major: 5
    minor: 3
    patch: 4
```

The goal is to compile once, and run on every platform!

## Direct API Usage

The ne-API can also be used independently from the sandbox/portable executable in the same way you would use frameworks like SDL. To use **ne** as a library, download the repository and build with [CMake](https://cmake.org/cmake-tutorial/) using the command line:

```
mkdir build
cd build
cmake ..
```
A `.lib` or `.o` will be output for the platform you are targeting in the `lib` folder. Link that directly into your application set your includes to point at the `include` folder. Note that because **ne** is a standard specification there may be multiple implementations you can use by different vendors, which may or may not support the library form as mentioned here. The example implementations we provide here will always allow this usage.

## Supported Concepts

- Window
- Clipboard
- Keyboard
- Mouse
- Touch
- Gamepads
- Joysticks
- Force Feedback
- Sensors
- Location
- Timers
- Sockets
- File IO
- File System
- Directory Watcher
- User Profile
- Contacts
- Calendar
- Shared Objects
- Processes
- Threads
- Synchronization
- Atomics
- Intrinsics
- Debugger
- Camera
- Printer
- USB
- Com/Serial Port
- Parallel Port
- Bluetooth
- Phone Calls
- Text Messages
- Network
- Power
- Audio
- Video
- Display
- GPU
- Packages

**Anything that can be written in language is NOT included as part of the ne-API.**

For example, regular expressions can be entirely defined in language and don’t require any operating system functionality, so they are NOT included in the ne-API and instead could be available as a package. Even functions as tiny as getting an error string from an ne-API error code is NOT included in the ne-API because it can technically be defined once in a library, rather than in every platform implementation.

When compiling against the ne-API in languages like C / C++, the standard libraries are not implicitly available (libc, libc++, STL, etc). Instead, these libraries can be downloaded as packages.

## Comparisons

#### SDL
**ne** mirrors some functionality of SDL, however it also adds more such as port communication, Bluetooth, sockets, file system, etc. **ne** is also a standard, rather than an implementation. In fact, SDL can be used to implement part of the ne-API for platforms that SDL supports. SDL acts only as a library that each platform must compile and link in. With **ne** you only compile code once to LLVM byte-code, and then it executes on every platform and auto-magically links against the ne-API at runtime. The **ne** specification also includes details for a secure sandbox environment which allows applications to be safely run.

#### WebAssembly
The closest comparison to **ne**, except that it requires running within a browser environment. Native applications must link in a layer of glue that binds all functions to the browser's JavaScript implementations. **ne** aims to be a truly native application with a very low level and laser focused API, unlike the bloated standard that is the web. For example, raw sockets aren't yet available for WebAssembly applications, only WebSockets due to lack of browser implementation. The ne-API does not require platforms to implement **anything** that can be written purely in LLVM byte-code. For example, web requests are not part of the ne-API because they can be written with the ne_socket API. To make web-requests, you would grab a web-request library built for **ne** using the package manager.

#### PNaCl
Much of the ne-sandbox is based on research that Google's NaCl / PNaCl pioneered. PNaCl was based on LLVM byte-code and also had an extensive API. Unfortunately, PNaCl has since disbanded due to low levels of adoption, mostly tied to the browser wars. There were also complaints that PNaCl's API was tied to Chrome implementations. **ne** aims to be a standard rather than an implementation, and is independent of any corporation or browser.

#### POSIX
The POSIX standard provided primitives such as threads, pipes, processes, timers, port I/O, etc. It's aim was to provide a common standard for Unix based operating systems. In that way **ne** is very similar with more primitives to match modern functionality. However, **ne** is also does not cover any functionality that could be implemented entirely as a library, such as POSIX regular expressions. In **ne**, regular expressions would be implemented as a package library, rather than as part of the API. This makes the ne-API more concise and easier to implement on new platforms. Imagine for example attempting to implement an entire web-browser; the standard is so large that it would take years for a single person to attempt a complete implementation. The ne-API can be implemented in a matter of weeks.

## Authors

* **Trevor Sundberg** - *Initial work* - [LinkedIn](https://www.linkedin.com/in/trevor-sundberg/)

See also the list of [contributors](https://github.com/TrevorSundberg/ne/contributors) who participated in this project.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Acknowledgments

This project is built on the shoulders of giants. We are forever thankful to them for their contributions:
* [PNaCl](https://developer.chrome.com/native-client/nacl-and-pnacl)
* [SDL](https://www.libsdl.org/index.php)
* [WebAssembly](https://webassembly.org/)
* [POSIX](https://en.wikipedia.org/wiki/POSIX)
* [LLVM](https://llvm.org/)
* [Vulkan](https://www.khronos.org/vulkan/)
* [npm](https://www.npmjs.com/)
