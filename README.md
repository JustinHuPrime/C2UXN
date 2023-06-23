# C2UXN

A (subset of) C18 to UXN compiler

See <https://wiki.xxiivv.com/site/uxn.html> for more about UXN.

## Unsupported Features

Given the limited nature of the UXN processor, the following standard-required C features are not supported:

- `_Thread_local` storage duration
- `unsigned` numbers (and their limits as defined in `<limits.h>` and the fixed-width unsigned types defined in `<stdint.h>`)
- floating point numbers (and the `<float.h>` header)
- minimum requirements for magnitude of integer limits (specifically, )

Furthermore:

- `sizeof(char) == 1` and `sizeof(short int) == sizeof(int) == sizeof(long int) == sizeof(long long int) == sizeof(void *) == 2`
- no fixed-width integer types of size 32 or 64 bits are available

## Environment

The compiler operates with a source character set of 8-bit ASCII (excluding anything below 0x20 (a.k.a. space) except for 0x12 (a.k.a. line feed, newline) and 0x11 (a.k.a. tab)) and an execution character set of 8-bit ASCII, and tries to be a conforming freestanding implementation.
