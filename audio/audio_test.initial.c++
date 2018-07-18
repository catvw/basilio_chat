// #include "core_audio.h++"
#include "portaudio/portaudio.h"
#include <iostream>
#include <exception>
#include <string>

/**
 * General PortAudio exception class, thrown when there's an error initializing
 * or terminating the library.
 * 
 * @author Charles Van West
 * @version 0.0
 */
class PortAudio_Exception : public std::exception {
public:
    PortAudio_Exception(const PaError err) : error(Pa_GetErrorText(err)) { }
    PortAudio_Exception(const std::string& err) : error(err) { }
    std::string get_error() const { return error; }
private:
    std::string error;
};

/**
 * More specific PortAudio exception class, thrown when something goes wrong
 * while using the library.
 * 
 * @author Charles Van West
 * @version 0.0
 */
class PortAudio_Use_Exception : public PortAudio_Exception {
public:
    using PortAudio_Exception::PortAudio_Exception; /* inherit constructors */
};

/**
 * Structure to pass a static data field to the callback function
 */
struct pa_test_data { float left_phase, right_phase; };

/**
 * Callback function for testing out PortAudio. Parameters should be pretty
 * self-explanatory.
 */
static int pa_test_callback(const void* input_buffer, void* output_buffer,
                            unsigned long int buffer_frames,
                            const PaStreamCallbackTimeInfo* time_info,
                            PaStreamCallbackFlags status_flags,
                            void* user_data) {
    /* cast data into a reasonable structure */
    pa_test_data* data = static_cast<pa_test_data*>(user_data);
    const float* in  = static_cast<const float*>(input_buffer);
    float* out = static_cast<float*>(output_buffer);
    
    for (unsigned long int i = 0; i < buffer_frames; ++i) {
        /* read left and right phase from input buffer into data, then shift */
        data->left_phase  = *(in++);
        data->right_phase = data->left_phase;
        
        /* put left and right phase into output buffer, then shift by two */
        out[0] = data->left_phase;
        out[1] = data->right_phase;
        out += 2;
    }
    
    return 0;
}

int main() {
    static constexpr int SAMPLE_RATE = 44100;
    static pa_test_data data;
    
    try {
        PaError error;
        
        /* initialize PortAudio */
        error = Pa_Initialize();
        if (error != paNoError) {
            throw PortAudio_Exception(error);
        } else {
            std::cout << "Initialized PortAudio successfully." << std::endl;
        }
        
        /* open a stream */
        PaStream* stream;
        error = Pa_OpenDefaultStream(&stream,
                                     1,          /* mono input */
                                     2,          /* stereo output */
                                     paFloat32,  /* 32 bit floating point 
                                                    samples */
                                     SAMPLE_RATE,
                                     paFramesPerBufferUnspecified, /* frames per
                                             buffer, i.e. the number
                                             of sample frames that PortAudio
                                             will request from the callback.
                                             Many apps may want to use
                                             paFramesPerBufferUnspecified, which
                                             tells PortAudio to pick the best,
                                             possibly changing, buffer size.*/
                                     pa_test_callback, /* the callback
                                                          function */
                                     &data); /* pointer that will be passed to
                                                the callback as user_data */
        if (error != paNoError) {
            throw PortAudio_Use_Exception(error);
        } else {
            std::cout << "Opened a stream." << std::endl;
        }
        
        /* start the stream */
        error = Pa_StartStream(stream);
        if (error != paNoError) {
            throw PortAudio_Use_Exception(error);
        } else {
            std::cout << "Initialized the stream." << std::endl;
        }
        
        /* sleep for some seconds */
        Pa_Sleep(10000);
        
        /* stop the stream */
        error = Pa_StopStream(stream);
        if (error != paNoError) {
            throw PortAudio_Use_Exception(error);
        } else {
            std::cout << "Stopped the stream." << std::endl;
        }
        
        /* close the stream */
        error = Pa_CloseStream(stream);
        if (error != paNoError) {
            throw PortAudio_Use_Exception(error);
        } else {
            std::cout << "Closed the stream." << std::endl;
        }
        
        /* terminate PortAudio */
        error = Pa_Terminate();
        if (error != paNoError) {
            throw PortAudio_Exception(error);
        } else {
            std::cout << "Terminated PortAudio successfully." << std::endl;
        }
    } catch (PortAudio_Use_Exception exc) {
        std::cout << "Error using PortAudio: "
                  << exc.get_error()
                  << std::endl;
        Pa_Terminate();
        return 1;
    } catch (PortAudio_Exception exc) {
        std::cout << "Error in PortAudio: "
                  << exc.get_error()
                  << std::endl;
        return 2;
    }
    
    return 0;
}
