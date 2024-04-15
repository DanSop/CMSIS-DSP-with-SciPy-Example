"""
Filename: Eq_SciPy_ARM.py
Author:   Danny Soppit
Description: This module provides filter coefficients for an audio
             equalizer. Various components of the filter can be changed.

"""

# ~~~~~~~~~~ Python Libraries ~~~~~~~~~~~~~

import cmsisdsp as dsp
import numpy as np
import librosa
import soundfile as sf
import matplotlib.pyplot as plt
from pylab import figure, plot, show
from scipy.signal import butter, sosfreqz, freqz, tf2zpk, zpk2sos, sosfilt
from matplotlib.ticker import ScalarFormatter

# ~~~~~~~~~~ Define Parameters ~~~~~~~~~~~~~

BASE_FREQUENCY      = 100         # Octave freq at which the first band center is started
NUM_BANDS           = 6           # Number of passband filters, you will see NUM_BANDS+1 a lot in the code as the first band has 0 as an edge frequency and is omitted
FS                  = 16000       # Hz sampling rate

SIG_BASE_FREQUENCY  = 80          # Main frequency for the generated input signal
SIG_NOISE_FREQUENCY = 2000        # Noise frequency for the generated input signal

POSTSHIFT           = 4           # Scales the input signal by 4^2 before processesing
NUMSTAGES           = 3           # Number of cascaded biquad filters applied to each band / Butterworth bandpass SOS order

GENERATE_SIGNAL     = True        # False for wav input, True for generated signal
LOG_SCALE_PLOT      = True        # True for a log plot of the filter freq resp, linear elsewise

FIG_WIDTH           = 12          # Width in inches
FIG_HEIGHT          = 6           # Height in inches
 
INPUT_FILENAME      = "input_file.wav"
SCIPY_OUT_FILENAME  = "SciPy-output_file.wav"
ARM_OUT_FILENAME    = "ARM-output_file.wav"

# ~~~~~~~~~~ Class Definitions ~~~~~~~~~~~~~

class SignalProcessor:
    def __init__(self):
        self.fs = FS
        self.base_frequency = BASE_FREQUENCY
        self.num_bands = NUM_BANDS + 1
        self.input_signal = None
        self.sos_list = []
        self.frequencies = []
        self.edges = []
        self.coefs = []
        
        return
        
    def generate_input_signal(self) -> None:  
    
        # Create a signal by having a base sinewave with additional noise superimposed upon it
        t = np.arange(0, 0.5, 1/self.fs) 
        freq = SIG_BASE_FREQUENCY  
        sine_wave = np.sin(2 * np.pi * freq * t)
        noise_frequency = SIG_NOISE_FREQUENCY  
        noise = 0.25 * np.sin(2 * np.pi * SIG_NOISE_FREQUENCY * t)
        self.input_signal = sine_wave + noise
        
        # Plot input signal
        plt.figure(figsize=(FIG_WIDTH, FIG_HEIGHT))
        
        plt.subplot(2, 1, 1) 
        plt.plot(t, self.input_signal, label='Generated Noise Signal')
        plt.title('Original Signal: Time Domain')
        plt.xlabel('Time (s)')
        plt.ylabel('Amplitude')
        plt.legend()
        
        plt.subplot(2, 1, 2)
        plt.magnitude_spectrum(self.input_signal, Fs=self.fs, scale='dB')
        plt.title('Original Signal: Frequency Domain')
        plt.xlabel('Frequency (Hz)')
        plt.ylabel('Magnitude (dB)')
        
        plt.tight_layout()
        
        return

    def load_input_signal(self, filename=INPUT_FILENAME) -> None:
    
        # Obtain the input signal
        input_filename = filename
        self.input_signal, fs = librosa.load(input_filename, sr=None)
        fs = self.fs

        # Plot input signal
        plt.figure(figsize=(FIG_WIDTH, FIG_HEIGHT))
        
        plt.subplot(2, 1, 1)
        plt.plot(np.arange(len(self.input_signal)) / fs, self.input_signal, label='Your Input Signal')
        plt.title('Input Signal: Time Domain')
        plt.xlabel('Time (s)')
        plt.ylabel('Amplitude')
        plt.legend()
        
        plt.subplot(2, 1, 2)
        plt.magnitude_spectrum(self.input_signal, Fs=fs, scale='dB')
        plt.title('Input Signal: Frequency Domain')
        plt.xlabel('Frequency (Hz)')
        plt.ylabel('Magnitude (dB)')
        
        plt.tight_layout()
        
        return
        
    def calculate_centers(self, base_frequency, num_bands):
    
        # Calculate the frequencies and frequency edges based upon the octave frequency and number of bands desired
        frequencies = [base_frequency * 2**i for i in range(-1, num_bands)]
        edges = [0] * (len(frequencies) - 1)

        for i in range(len(frequencies)-1):
            edges[i] = 10**(np.log10(frequencies[i]) + ((np.log10(frequencies[i+1]) - np.log10(frequencies[i])) / 2))
    
        # Print out the obtained frequency bands
        print(f"\nOctaves [Hz]\t\tEdge Frequencies [Hz]\n")
        for i, (freq, edge) in enumerate(zip(frequencies, edges)):
            if i == 0:
                print(f"Omitted band\n{freq:8.2f} [Hz]\t\t0\tand {edge:8.2f} [Hz]\n")
                print(f"Bands used:")
            else:
                print(f"{freq:8.2f} [Hz]\t\t{edges[i-1]:.2f}\tand {edge:8.2f} [Hz]")
        print(f"\n")
        
        self.frequencies = frequencies
        self.edges = edges  
        
        return frequencies, edges
        
    def plot_bandpass_filter_response(self, scale='linear'):
        plt.figure(figsize=(FIG_WIDTH, FIG_HEIGHT))
      
        # Obtain the freq, resp, and sos from the Butterworth bandpass filter
        for i in range(0, NUM_BANDS):
            lowcut = self.edges[i]
            highcut = self.edges[i + 1]

            freq, resp, sos = self.butter_bandpass(lowcut, highcut, self.fs, i, order=NUMSTAGES)
            self.sos_list.append(sos)
            
            # Scale the coefficients by the poststage factor and format to Q31
            coefs=np.reshape(np.hstack((sos[:,:3],-sos[:,4:])), 15)
            coefs = coefs / (POSTSHIFT ** 2)
            coefsQ31 = (coefs * (2**31)) 
            coefsQ31= np.round(coefsQ31)
            
            print("")
            print("~~~~~~~~~~ Scaled Q31 Biquad Coefficient bands: {:.1f} Hz to {:.1f} Hz: ~~~~~~~~~~ \n".format(lowcut, highcut))
            print(" ".join("{:.2f}".format(x) for x in coefsQ31))
            print("\n\n")
             
            plt.plot(freq, np.abs(resp))
            plt.title('Magnitude Response of Butterworth Bandpass Filter')
            plt.xlabel('Frequency (Hz)')
            plt.ylabel('Magnitude')

        # Plot the signal in log if desired
        if LOG_SCALE_PLOT:
            plt.semilogx(self.frequencies, np.ones_like(self.frequencies), 'o', label='Centres of the bandpass filters')
            plt.semilogx(self.edges, np.ones_like(self.edges) * 1, '*', label='Edges of the bandpass filters')
            plt.title('Logarithmic Scale Plot of Frequencies')
            plt.gca().xaxis.set_major_formatter(ScalarFormatter())
            plt.legend()
            plt.grid(True)
   
        return
            
    def butter_bandpass(self, lowcut, highcut, fs, i, order): 
    
        # Calculate nyquist freq and the upper bounds of the bandpass filter
        nyquist = 0.5 * fs
        low = lowcut / nyquist
        high = highcut / nyquist
        
        # Obtain the a & b coefficients for each filter 
        # 5 coefficients for each filter * number of stages/order
        b, a = butter(order, [low, high], btype='band', analog=False)
        
        # Computes the frequency response of a digital filter
        frequencies, response = freqz(b, a, fs=fs)
        
        # Obtain the zero, pole, and gain representation of a digital filter
        z, p, k = tf2zpk(b, a)
        
        # Return the second-order sections from zeros, poles, and gain of a system
        sos = zpk2sos(z, p, k)
     
        return frequencies, response, sos   
        
    def apply_filters_and_print_python(self):
    
        # Filter the signal using a digital IIR filter defined by sos.
        signal_list = [sosfilt(sos, self.input_signal) for sos in self.sos_list]

        # Scale the bands here, for example the first band scaled by a factor of 1.
        # This is where the "equalization" portion would be applied to tune the bands
        signal_list[0] *= 1
        
        # Sum up the signals to reconstruct the signal
        final_signal = np.sum(signal_list, axis=0)

        # Output the signal to a wav file
        output_filename = "filtered_output.wav"
        sf.write(output_filename, final_signal, self.fs)

        # Plot resulting signal
        plt.figure(figsize=(FIG_WIDTH, FIG_HEIGHT))
        
        plt.subplot(2, 1, 1)
        plt.plot(np.arange(len(final_signal)) / self.fs, final_signal, label='SciPy Filtered Signal')
        plt.title('Python SciPy: Time Domain for the Filtered Signal')
        plt.xlabel('Time (s)')
        plt.ylabel('Amplitude')
        plt.legend()
        
        plt.subplot(2, 1, 2)
        plt.magnitude_spectrum(final_signal, Fs=self.fs, scale='dB')
        plt.title('Python SciPy: Frequency Domain for the Filtered Signal')
        plt.xlabel('Frequency (Hz)')
        plt.ylabel('Magnitude (dB)')
        
        plt.tight_layout()
        
        output_filename = SCIPY_OUT_FILENAME    
        sf.write(output_filename, final_signal, self.fs)
        
        return
        
    def apply_filters_and_print_ARM(self):
    
        # Local variables
        signal_ARM = []
        
         # Loop over the number of number of frequency bands
        for i in range(0, NUM_BANDS):
            sos = self.sos_list[i]
            
            # Reshape the sos and scale the coefficents down based off of the postshift
            coefs=np.reshape(np.hstack((sos[:,:3],-sos[:,4:])), NUMSTAGES * 5)
            coefs = coefs / (POSTSHIFT ** 2)
            
            # Convert the coefficients to Q31 
            coefsQ31 = (coefs * (2**31)) 
            coefsQ31= np.round(coefsQ31)
            self.coefs = coefsQ31
        
            # Initialize the biquad filter and apply the filter
            state = np.zeros(NUMSTAGES * 4)
            biquadQ31 = dsp.arm_biquad_casd_df1_inst_q31()
            dsp.arm_biquad_cascade_df1_init_q31(biquadQ31, NUMSTAGES, self.coefs, state, POSTSHIFT)

            # Convert the signal to Q31 and scale it down for filtering
            sigQ31 = self.input_signal * (2**31)
            sigQ31 = sigQ31 / 4

            # Apply the filter
            res2 = dsp.arm_biquad_cascade_df1_q31(biquadQ31, sigQ31) 

            # Scale the signal back up and reconvert it back
            res2 *= 4
            res2 = res2 / (2**31)

            # Append this new signal to an array
            signal_ARM.append(res2)

        # Scale the bands here, for example the first band scaled by a factor of 1.
        # This is where the "equalization" portion would be applied to tune the bands
        signal_ARM[0] *= 1 
        
        # Sum up all the signals together to reconstruction the original signal
        final_signal_ARM = np.sum(signal_ARM, axis=0)

        # Output the file name
        output_filename = ARM_OUT_FILENAME
        sf.write(output_filename, final_signal_ARM, self.fs)

        # Plot resulting signal
        plt.figure(figsize=(FIG_WIDTH, FIG_HEIGHT))
        
        plt.subplot(2, 1, 1)
        plt.plot(np.arange(len(final_signal_ARM)) / self.fs, final_signal_ARM, label='ARM Filtered Signal')
        plt.title('ARM: Time Domain for the Filtered Signal')
        plt.xlabel('Time (s)')
        plt.ylabel('Amplitude')
        plt.legend()
        
        plt.subplot(2, 1, 2)
        plt.magnitude_spectrum(final_signal_ARM, Fs=self.fs, scale='dB')
        plt.title('ARM: Frequency Domain for the Filtered Signal')
        plt.xlabel('Frequency (Hz)')
        plt.ylabel('Magnitude (dB)')
        
        plt.tight_layout()
        
        return
        
if __name__ == "__main__":

    # ~~~~~~~~~~ Signal Generation ~~~~~~~~~~~~~

    if GENERATE_SIGNAL:
        processor = SignalProcessor()
        processor.generate_input_signal()
    else:
        processor = SignalProcessor()
        processor.load_input_signal(filename=INPUT_FILENAME)   
       
    # ~~~~~~~~~~ Filter Generation ~~~~~~~~~~~~~

    processor.calculate_centers(BASE_FREQUENCY, NUM_BANDS+1)
    processor.plot_bandpass_filter_response()
       
    # ~~~~~~~ Python Filter Application ~~~~~~~~

    processor.apply_filters_and_print_python()

    # ~~~~~~~~~ ARM Filter Application ~~~~~~~~~

    processor.apply_filters_and_print_ARM()

    # ~~~~~~~~~~~~ Show the plots ~~~~~~~~~~~~~~

    plt.show()
