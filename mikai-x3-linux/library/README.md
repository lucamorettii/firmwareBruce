# LibMIKAI

## Introduction
MIKAI is a recent project that aims to avoid the sale of mykey program, making it **free** and **open source**.

**Donations are accepted** (only cryptocurrencies).
Current maintainer is [Lilz0C](https://t.me/Lilz0C).
This library has been completely **rewritten** by him.

**LICENSE:** [GNU LGPL v3.0](https://www.gnu.org/licenses/lgpl-3.0.html)

## Library structure
| Layer | Description                      |
|-------|----------------------------------|
| 4     | Public API (mikai.h)             |
| 3     | Application level (mykey folder) |
| 2     | SRIX4k commands (srix4k folder)  |
| 1     | ISO1443b-ST management (libnfc)  |
| 0     | Physic level (PN532)             |

## Getting started
Make sure that CMake is installed on your PC and run it selecting a target folder. 

You have to compile [libnfc](https://github.com/nfc-tools/libnfc) yourself as static library and put the resulting libnfc.a file in the target build folder created previously (automatic linkage).

Run the compilation with GCC.