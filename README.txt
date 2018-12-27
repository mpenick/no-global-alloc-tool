# To run

```
./no-global-alloc-tool /path/to/source/cluster.cpp -- -I/some/include/directory
In file included from .../Code/cpp-driver/src/cluster.cpp:17:
In file included from .../Code/cpp-driver/src/cluster.hpp:20:
In file included from .../Code/cpp-driver/src/config.hpp:20:
In file included from .../Code/cpp-driver/src/auth.hpp:22:
In file included from .../Code/cpp-driver/src/host.hpp:22:
.../Code/cpp-driver/src/copy_on_write_ptr.hpp:85:7: error: Using `operator delete()` from /usr/lib/gcc/x86_64-linux-gnu/7.3.0/../../../../include/c++/7.3.0/new:124:6
      delete ref;
      ^
.../Code/cpp-driver/src/copy_on_write_ptr.hpp:74:54: error: Using `operator new()` from /usr/lib/gcc/x86_64-linux-gnu/7.3.0/../../../../include/c++/7.3.0/new:120:7
      ptr_ = SharedRefPtr<Referenced>(new Referenced(new T(*(temp->ref))));
                                                     ^
.../Code/cpp-driver/src/copy_on_write_ptr.hpp:85:7: error: Using `operator delete()` from /usr/lib/gcc/x86_64-linux-gnu/7.3.0/../../../../include/c++/7.3.0/new:124:6
      delete ref;
```

*Note:* Includes and other compiler flags can be specified after `--`

# Dependencies

Requires LLVM and clang (w/ clang's libtooling)

## On Ubuntu

```
sudo apt install libclang-6.0-dev
```

# To build

```
mkdir build && cd build
LIBCLANG_ROOT_DIR=/path/to/llvm/clang cmake ..
make
```
