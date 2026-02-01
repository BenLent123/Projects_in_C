# Ncrypt 64Bit RSA encryption

This is an exploration into secure encryption via RSA in C, this is mainly done for education but as for as it matters this does actually work.

## Features

- 32bit prime generation via primality tests
- 64bit RSA modulus
- Encryption test of random ASCII

## Requirements

- gcc/clang for uint128

## Usage

```bash
gcc -Wall -o ncrypt ncrypt.c
./ncrypt