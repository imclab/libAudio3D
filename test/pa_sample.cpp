
#include <assert.h>
#include <stdio.h>
#include <vector>

#include <termios.h>
#include <unistd.h>

#include "audio_render.h"
#include "room_model.h"

#include "portaudio.h"

const int kSampleRate = 44100;
const int kFramesPerBuffer = 256;

static Point3D source_pos = {0.5, 0.5, 0.5};

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
    AudioRender* audio_render = reinterpret_cast<AudioRender*>(userData);
    assert(audio_render!=0);

    audio_render->SetSourcePosition(source_pos);

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
    audio_render->RenderAudio(input, &output_left, &output_right);

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

    RoomModel room_model;
    room_model.DefineBox(5, 2, 3, 0.7);

    AudioRender audio_render(config, &room_model);

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
              &audio_render);
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
    	case 'x':
    		source_pos.x -= 0.1;
    		break;
    	case 'X':
    	  source_pos.x += 0.1;
    		break;
    	case 'y':
    	  source_pos.y -= 0.1;
    		break;
      case 'Y':
        source_pos.y += 0.1;
        break;
      case 'z':
         source_pos.z -= 0.1;
         break;
      case 'Z':
         source_pos.z += 0.1;
         break;
    	default:
    		break;
    	}
    	std::cout<<"Source Pos: "<<source_pos<<std::endl;
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
