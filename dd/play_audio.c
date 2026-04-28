#include <stdio.h>          // Standard I/O library for file operations (fopen, fread, fclose, printf, perror)
#include <stdlib.h>         // Standard library for general utilities (exit, malloc, free)
#include <alsa/asoundlib.h> // ALSA library for audio playback and recording on Linux

#define AUDIO_FILE "/home/mirafra/Downloads/audio_testing.wav"  // Name of the WAV file to play

int main() {
    snd_pcm_t *pcm;               // Pointer to an ALSA PCM device handle
    FILE *fp = fopen(AUDIO_FILE, "rb");  // Open WAV file in binary read mode
    if (!fp) { perror("fopen"); return 1; } // Error check: if file can't be opened, print error and exit

    fseek(fp, 44, SEEK_SET);      // Skip the WAV header (44 bytes) to start reading raw PCM data

    // Open ALSA PCM device for playback
    snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);

    // Set ALSA PCM parameters:
    // - format: signed 16-bit little endian (S16_LE)
    // - access type: interleaved (samples for multiple channels stored alternately)
    // - channels: 2 (stereo)
    // - sample rate: 44100 Hz
    // - soft resample: 1 (allow software resampling)
    // - latency: 500000 microseconds (~0.5 seconds)
    snd_pcm_set_params(pcm,
        SND_PCM_FORMAT_S16_LE,
        SND_PCM_ACCESS_RW_INTERLEAVED,
        2, 44100, 1, 500000);

    char buffer[4096];   // Temporary buffer to hold PCM data read from WAV
    int frames;          // Number of bytes read from WAV file

    // Loop: read PCM data from file and send to ALSA for playback
    while ((frames = fread(buffer, 1, sizeof(buffer), fp)) > 0)
        // Write frames to PCM device
        // Divide by 4 because:
        // - 2 channels (stereo) × 2 bytes per sample (16-bit) = 4 bytes per frame
        snd_pcm_writei(pcm, buffer, frames / 4);

    snd_pcm_drain(pcm);  // Wait for all remaining samples to finish playing
    snd_pcm_close(pcm);  // Close ALSA PCM device
    fclose(fp);          // Close WAV file
    return 0;            // Exit program successfully
}

