# LibMIKAI

## Introduction
MIKAI is a recent project that aims to avoid the sale of mykey program, making it **free** and **open source**.

**Donations are accepted and needed for the development of this project!** (only cryptocurrencies).

Bitcoin: bc1q6anwfehg99uymlgevh9enes5t674etjm85k8jd
Ethereum: 0x11d7D21B5715fD1AAB005a645614209295CA2fCE
Monero: 44meLyqcheWRUSdxsiXA5b4ZRKidCtangFTS6orM5wBu8SbYYtGhjZDMyEyBS5WT3PKs73V9ZMzCvavN9p8sdzVQJoXx56x

Current maintainer is [Lilz0C](https://t.me/Lilz073).
This library has been completely **rewritten** by him.

## LICENSE:
libmikai: Proprietary license (primary, LICENSE.md file) and [GNU LGPL v3.0](https://www.gnu.org/licenses/lgpl-3.0.html) (provided that the conditions of the first license are respected)
mikai compiled: Proprietary license (LICENSE.md file)

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