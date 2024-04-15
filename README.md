# CMSIS-DSP & SciPy IIR Coefficients Generation and Testing
## Information.

This project uses SciPy and CMSIS-DSP libraries to calculate IIR filter coefficients. These coefficients are printed and can be implemented in any C/C++ project that utilizies the ARM CMSIS DSP library. The script uses Butterworth bandpass filters to create a series of filters for a filter bank. The signals then are added up at the end to reconstruct the signal. The block diagram is shown below:

![block](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/1a3f8a06-cbf2-4f57-9fff-1c9f594ab845)

The C file, Eq_ARM.c, shows an example of how to apply the IIR filter using the coefficients. This file is generic and does not include data obtaining or streaming.
The Python file, Eq_SciPi_ARM.py, shows how the coefficients are generating alongside applying the coefficients via SciPy and a ARM CMSIS-DSP library which is a direct wrapper to the C library. The Python code is in a single file for simplicity.

SciPy: https://docs.scipy.org/doc/scipy/
CMSIS-DSP : https://www.keil.com/pack/doc/CMSIS/DSP/html/index.html

## Installation and Usage

pip install -r requirements.txt
Tested with Python 3.11.x. as the distutils package is removed in python version 3.12. A work around for this version is cited in "https://stackoverflow.com/questions/69919970/no-module-named-distutils-but-distutils-installed".
Note you will need some Wav input file named "input_file.wav" for testing

## Example Plots

The input signal:
![input](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/02bbb291-6de3-4890-8a17-ff998d4c9187)

The filter being apply via SciPy:
![scipy](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/51fdc9af-e16b-4781-a17a-e7e2952b3b0b)

The filter being apply via the ARM CMSIS-DSP library:
![arm](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/5a25c598-84c2-4d1b-bd12-e11475fe2272)

The logarithmic and linear plots of the filter created:
![log-filter](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/3405ecaf-2bd8-4d61-9314-4c01c439ec17)

![linear](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/21bb3913-c383-43aa-ad53-e26dda2faf68)
