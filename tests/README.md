# Tests

Supertux comes with 2 types of tests.

- **Unit tests** - Generic, "build-time" tests that are done without any user interaction.
- **Game tests** - Specialized tests using the built-in test menu in the game.

Unit tests are more traditional, but game tests allow for developers
to flexibly test game logic, physics, menus, etc. However, these tests
are built into the Supertux binary itself, instead of as separate unit
test files.

So for developers, it's simple: If you need to test a library,
function, or anything that could potentially change, use unit
tests. If you need to test physics, run benchmark tests, test GUI
elements, or game input; use game tests.

## Hierarchy

- **[`data/`](data/)**: Data files needed to perform any tests. These are also used by game tests.
- **[`unit/`](unit/)**: Unit test files designed to fully test a single specific file in the [src](../src/) folder at the root of the repository. The folder structure and file naming should be identical in both folders.


