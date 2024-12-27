#!/bin/bash

# Function to clear executables and test.wav
clear_files() {
    if [ -f "./modulate" ]; then
        rm ./modulate
        echo "Executable 'modulate' has been removed."
    else
        echo "Executable 'modulate' does not exist."
    fi

    if [ -f "./demodulate" ]; then
        rm ./demodulate
        echo "Executable 'demodulate' has been removed."
    else
        echo "Executable 'demodulate' does not exist."
    fi

    if [ -f "./test.wav" ]; then
        rm ./test.wav
        echo "File 'test.wav' has been removed."
    else
        echo "File 'test.wav' does not exist."
    fi
}

# Function to compile and run modulate
run_modulate() {
    gcc modulate.c -o modulate -ljpeg -lm
    if [ $? -eq 0 ]; then
        echo "Compilation of 'modulate.c' successful. Running ./modulate..."
        ./modulate
    else
        echo "Compilation of 'modulate.c' failed. Please check for errors."
        exit 1
    fi
}

# Function to compile and run demodulate
run_demodulate() {
    gcc demodulate.c -o demodulate -ljpeg -lm
    if [ $? -eq 0 ]; then
        echo "Compilation of 'demodulate.c' successful. Running ./demodulate..."
        ./demodulate
    else
        echo "Compilation of 'demodulate.c' failed. Please check for errors."
        exit 1
    fi
}

# Function to transmit (run modulate, wait 500ms, then run demodulate)
run_transmit() {
    run_modulate
    echo "Waiting 500ms before running demodulate..."
    sleep 0.5
    run_demodulate
}

# Check for options
case "$1" in
    -mod)
        run_modulate
        ;;
    -demod)
        run_demodulate
        ;;
    -transmit)
        run_transmit
        ;;
    -clear)
        clear_files
        ;;
    *)
        echo "Usage: $0 {-mod|-demod|-transmit|-clear}"
        echo "  -mod      : Compile and run modulate.c"
        echo "  -demod    : Compile and run demodulate.c"
        echo "  -transmit : Compile and run modulate.c, wait 500ms, then run demodulate.c"
        echo "  -clear    : Remove all executables and test.wav"
        exit 1
        ;;
esac
