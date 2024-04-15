# CMSIS-DSP & SciPy IIR Coefficients Generation and Testing
## Information.

This project uses SciPy and the CMSIS-DSP library to calculate IIR filter coefficients. These coefficients are printed and can be implemented in any C/C++ project that utilizies the ARM CMSIS DSP library. The script uses Butterworth bandpass filters to create a filter bank. Each signal is added up at the end to reconstruct the original signal with gain modifiers if desired. The block diagram is shown below:

![block](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/8bad7f36-25a3-4dff-9c27-5cbdb35f849a)

The C file, Eq_ARM.c, shows an example of how to apply the IIR filter using the coefficients. This file is generic and does not include data obtaining or streaming.
The Python file, Eq_SciPi_ARM.py, shows how the coefficients are generating alongside applying the coefficients via SciPy and an ARM CMSIS-DSP library which is a direct wrapper to the C library. The Python code is in a single file for simplicity.

The goal here is to have the SciPy and CMSIS-DSP plots to "mirror" one another to ensure the filters are being applied properly. It essentially provides a quick way to generate a filter bank, test the filter bank with any signal, and copy the generated coefficients in the Q31 format to any external project utizling CMSIS-DSP. The scripts and filters, of course, can be editted and used however needed.

SciPy: https://docs.scipy.org/doc/scipy/  
CMSIS-DSP : https://www.keil.com/pack/doc/CMSIS/DSP/html/index.html

## Installation and Usage

To install the packges: ```pip install -r requirements.txt```  

To launch the Python script: ```python Eq_SciPy_ARM.py``` or ```python3 Eq_SciPy_ARM.py```  

Tested with Python 3.11.x. as the distutils package is removed in python version 3.12. A work around for this version is cited in "https://stackoverflow.com/questions/69919970/no-module-named-distutils-but-distutils-installed".
Note you will need some Wav input file named "input_file.wav" for testing if the Wav input is selected as True. A default wav file was included.  

If you wish to apply gains to the signals before they are reconstructed, line 206 & 277 in Eq_SciPy_ARM.py show an example of gaining the first band (signal_list[0]) by a factor of 1. Add more and change the gain factor as needed.

## Example Plot with a 16 kHz Wav file input - no filtering. 

The input signal:

![input](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/02bbb291-6de3-4890-8a17-ff998d4c9187)

The filter being apply via SciPy:  

![scipy](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/51fdc9af-e16b-4781-a17a-e7e2952b3b0b)

The filter being apply via the ARM CMSIS-DSP library:  

![arm](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/5a25c598-84c2-4d1b-bd12-e11475fe2272)

The logarithmic plot of the filter bank. Note that the edge and center frequencies for the bands not calculated are shown:  

![log-filter](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/3405ecaf-2bd8-4d61-9314-4c01c439ec17)

The linear plot of the filter bank:  

![linear](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/21bb3913-c383-43aa-ad53-e26dda2faf68)


## Example Plot with a generated signal input - filtering. 

The signal input is a simple sign wave with a base frequency of 80 Hz and a noise frequency at 2 kHz. Every band except the band containing 80 Hz is attenutately heavily.

The original signal plot:  

![forig](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/02edb752-ad29-4940-b4e9-7977fe58e25e)

The signal filtered via SciPy:  

![fscipy](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/75407e3f-5363-412b-a78e-33338fe7b55f)

The signal filtered via the ARM CMSIS-DSP library:   

![farm](https://github.com/DanSop/CMSIS-DSP-with-SciPy-Example/assets/55635377/aad506cf-ce93-446b-abf2-62a1cc78e135)
