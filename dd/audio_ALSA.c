#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define PCM_DEVICE "default"  // system default sound card

int main() {
    snd_pcm_t *capture_handle, *playback_handle;
    snd_pcm_hw_params_t *hw_params;
    int err;

    unsigned int rate = 44100;
    int channels = 2;   // stereo(2) and mono(1)
    snd_pcm_uframes_t frames = 1024;   // frame -> group of samples, 1024 frames per read/write

// Each sample is 16 bits(2bytes)

    int duration = 5; // seconds
    int total_frames = rate * duration;

    int16_t *buffer;
    buffer = malloc(frames * channels * sizeof(int16_t));  // used for both recording and playback

    printf("Opening capture device...\n");

    // Open capture device
    if ((err = snd_pcm_open(&capture_handle, PCM_DEVICE,
                            SND_PCM_STREAM_CAPTURE, 0)) < 0) {  // opens the PCM device
//SND_PCM_STREAM_CAPTURE  -->  record mode

        fprintf(stderr, "Cannot open capture device: %s\n", snd_strerror(err));
        return 1;
    }

    snd_pcm_hw_params_alloca(&hw_params);   // allocates structure on stack
    snd_pcm_hw_params_any(capture_handle, hw_params);  // fill with the default value
    snd_pcm_hw_params_set_access(capture_handle, hw_params,
                                 SND_PCM_ACCESS_RW_INTERLEAVED);  // Access type, audio data stored in L R L R ... interleaved format
    snd_pcm_hw_params_set_format(capture_handle, hw_params,
                                 SND_PCM_FORMAT_S16_LE);  // 16bit audio, Little endian

    snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &rate, 0);  // which will request 44100 Hz

    snd_pcm_hw_params_set_channels(capture_handle, hw_params, channels);  // stereo recording(2 channels)
    snd_pcm_hw_params(capture_handle, hw_params);  // Apply all configuration to hardware

    snd_pcm_prepare(capture_handle);  // prepare device for use

    printf("Recording for %d seconds...\n", duration);

    FILE *fp = fopen("recorded.raw", "wb");

    int frames_to_capture = total_frames / frames;

/* 44100 x 5sec =220500 frames
	220500/1024 = 215 loops*/

    for (int i = 0; i < frames_to_capture; i++) {
        err = snd_pcm_readi(capture_handle, buffer, frames);  // it will read the audiofrom mic into buffer
        if (err < 0)
            err = snd_pcm_recover(capture_handle, err, 0);  // This will handle the buffer overrun and underrun
        if (err < 0) {
            fprintf(stderr, "Read error: %s\n", snd_strerror(err)); 
            break;
        }
        fwrite(buffer, sizeof(int16_t), frames * channels, fp);  //  // It will store r    aw PCM data

    }

    fclose(fp);
    snd_pcm_close(capture_handle);   // close the capture

    printf("Recording complete.\n");

    // ================= PLAYBACK =================      same flow but reversed

    printf("Opening playback device...\n");

    if ((err = snd_pcm_open(&playback_handle, PCM_DEVICE,
                            SND_PCM_STREAM_PLAYBACK, 0)) < 0) {   // open the speaker for playback
        fprintf(stderr, "Cannot open playback device: %s\n", snd_strerror(err));
        return 1;
    }

    snd_pcm_hw_params_any(playback_handle, hw_params);
    snd_pcm_hw_params_set_access(playback_handle, hw_params,
                                 SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(playback_handle, hw_params,
                                 SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &rate, 0);
    snd_pcm_hw_params_set_channels(playback_handle, hw_params, channels);
    snd_pcm_hw_params(playback_handle, hw_params);

    snd_pcm_prepare(playback_handle);

    printf("Playing recorded audio...\n");

    fp = fopen("recorded.raw", "rb");

    while ((err = fread(buffer, sizeof(int16_t),
                        frames * channels, fp)) > 0) {

        int frames_read = err / channels;

        err = snd_pcm_writei(playback_handle, buffer, frames_read);
        if (err < 0)
            err = snd_pcm_recover(playback_handle, err, 0);
        if (err < 0) {
            fprintf(stderr, "Write error: %s\n", snd_strerror(err));
            break;
        }
    }

    fclose(fp);
    snd_pcm_drain(playback_handle);
    snd_pcm_close(playback_handle);

    free(buffer);

    printf("Playback complete.\n");

    return 0;
}
