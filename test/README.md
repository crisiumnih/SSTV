## Build and Run Script

This script (`build.sh`) compiles and runs the `modulate.c` and `demodulate.c` programs, and provides additional utility options.

### Usage

Run the script with one of the following options:

```bash
./build.sh [OPTION]
```
### Options

- ```-mod```:
    -   Compiles modulate.c and runs the executable ./modulate.

- ```-demod```:
    - Compiles demodulate.c and runs the executable ./demodulate.

- ```-transmit```:
    - Compiles both modulate.c and demodulate.c, runs ./modulate, waits , and then runs ./demodulate.

- ```-clear```:
    - Removes all executables (modulate, demodulate) and the test.wav file.

### Requirements

- libjpeg and math
