# Tests

You can enable tests with CMake using:

`$ cmake -DBUILD_TESTING=On ..`

The target `tests` will attempt to build all the tests then run ctest. If using the Makefile CMake backend, you would run this:

`$ make tests`

## Hierarchy

- **[`unit/`](unit/)**: Unit test files designed to fully test a single specific file in the [src](../src/) folder at the root of the repository. The folder structure and file naming should be identical in both folders.


