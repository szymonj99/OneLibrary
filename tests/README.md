# OneLibrary - Catch2 Tests
The tests for this library are split into two sets; [manual](./manual) and [automated](./automated).

## Automated Tests
These tests are executed when running GitHub Actions. The tests in this set do not require either administrative privileges or mouse/keyboard/monitor to be connected.
These can also run on a local machine.

## Manual Tests
// TODO: Investigate if these could run on the likes of GitHub Actions or if we could host them in our own `worker`.
These tests need to be executed manually on the user's machine with `sudo` or Administrator privileges. These tests are used for keyboard and mouse functionality, as well as the server-client communication.
