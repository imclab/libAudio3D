
#include <assert.h>
#include <iostream>
#include <vector>

#include <termios.h>
#include <unistd.h>

#include "audio_3d.h"

#include "portaudio.h"

const int kSampleRate = 44100;
const int kFramesPerBuffer = 512;

static float elevation_deg = 0;
static float azimuth_deg = 0;

static int AudioCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
    float *out = (float*)outputBuffer;
    const float *in = (const float*)inputBuffer;
    unsigned int i;
    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    Audio3DSource* audio_3d = reinterpret_cast<Audio3DSource*>(userData);
    assert(audio_3d!=0);

    audio_3d->SetDirection(elevation_deg, azimuth_deg, 1.0);

    std::vector<float> input(framesPerBuffer, 0.0f);
    if( inputBuffer != 0 )
    {
        for( i=0; i<framesPerBuffer; i++ )
        {
        	float left_sample = (*in++);
        	float right_sample = (*in++);
        	input[i] = (left_sample + right_sample) / 2.0;  // Mono downmix
        }
    }

    std::vector<float> output_left;
    std::vector<float> output_right;
    audio_3d->ProcessBlock(input, &output_left, &output_right);

    assert(output_left.size()==framesPerBuffer);
    assert(output_right.size()==framesPerBuffer);
    for (int i=0; i<framesPerBuffer; ++i) {
    	(*out++) = output_left[i];
    	(*out++) = output_right[i];
    }

    return paContinue;
}

int main(void)
{
    PaStreamParameters inputParameters, outputParameters;
    PaStream *stream;
    PaError err;

    bool keep_running = true;
	Audio3DSource audio_3d(kSampleRate, kFramesPerBuffer);
	audio_3d.SetDirection(0,0, 10);

    err = Pa_Initialize();
    if( err != paNoError ) goto error;

    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
      fprintf(stderr,"Error: No default input device.\n");
      goto error;
    }
    inputParameters.channelCount = 2;       /* stereo input */
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
      fprintf(stderr,"Error: No default output device.\n");
      goto error;
    }
    outputParameters.channelCount = 2;       /* stereo output */
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              &outputParameters,
              kSampleRate,
              kFramesPerBuffer,
              paClipOff,
              AudioCallback,
              &audio_3d);
    if( err != paNoError ) goto error;

    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;



    printf("Hit ENTER to stop program.\n");
    // Black magic to prevent Linux from buffering keystrokes.
        struct termios t;
        tcgetattr(STDIN_FILENO, &t);
        t.c_lflag &= ~ICANON;
        tcsetattr(STDIN_FILENO, TCSANOW, &t);

    keep_running = true;
    while (keep_running) {
    	int c = getchar();
    	switch (c) {
    	case 27:
    		keep_running = false;
    		break;
    	case 'u':
    		elevation_deg += 5;
    		break;
    	case 'd':
    		elevation_deg -= 5;
    		break;
    	case 'l':
    		azimuth_deg -= 5;
    		break;
    	case 'r':
			azimuth_deg += 5;
			break;
    	default:
    		break;
    	}
    	std::cout<<"Elevation: "<<elevation_deg<<" Azimuth: "<<azimuth_deg<<std::endl;
    }
    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;

    printf("Finished");
    Pa_Terminate();
    return 0;

error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return -1;
}
