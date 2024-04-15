# CMSIS-DSP & SciPy IIR Coefficients Generation and Testing
## Information.

This project uses SciPy and CMSIS-DSP libraries to calculate IIR filter coefficients. These coefficients are printed and can be implemented in any C/C++ project that utilizies the ARM CMSIS DSP library.

The C file, Eq_ARM.c, shows an example of how to apply the IIR filter using the coefficients. This file is generic and does not include data obtaining or streaming.
The Python file, Eq_SciPi_ARM.py, shows how the coefficients are generating alongside applying the coefficients via SciPy and a ARM CMSIS-DSP library which is a direct wrapper to the C library. The Python code is in a single file for simplicity.

SciPy: https://docs.scipy.org/doc/scipy/
CMSIS-DSP : https://www.keil.com/pack/doc/CMSIS/DSP/html/index.html

## Installation and Usage

pip install -r requirements.txt
Tested with Python 3.11.x. as the distutils package is removed in python version 3.12. A work around for this version is cited in "https://stackoverflow.com/questions/69919970/no-module-named-distutils-but-distutils-installed".
Note you will need some Wav input file named "input_file.wav" for testing

## Example Plots