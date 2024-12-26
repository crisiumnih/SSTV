#!/bin/bash

# Compile modulate.c with required libraries
gcc modulate.c -o modulate -ljpeg -lm

# Check if the compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Executable created: ./modulate"
else
    echo "Compilation failed. Please check for errors."
    exit 1
fi

