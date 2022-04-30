# CPP MPC

Primarily a C++ rewrite of [SwiftMPC](https://github.com/ianruh/SwiftMPC) at
the moment. Not all tests have been implemented, but the core solver works.

In addition, the biggest improvement is runtime code generation and compilation
for the objective given the symbolic representation of the problem.

## Getting Started

Simply clone the repository, build, and run the tests. All dependencies except
Eigen are downloaded by CMake:

```
$ git clone git@github.com:ianruh/cppmpc.git
$ cd cppmpc/
$ ./utils build
$ ./utils test
```

### Adding to you project

You can add the following to you CMake project:

```
FetchContent_Declare(
    cppmpc
    GIT_REPOSITORY git@github.com:ianruh/cppmpc.git
)
FetchContent_MakeAvailable(cppmpc)
```

### Compilation Options

- `NO_VALIDATE_OBJECTIVE` Don't check the dimensions of the objective before 
  solving the problem. This can speed up the initialization time for the solver
