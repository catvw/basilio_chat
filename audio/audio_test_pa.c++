/**
 * Test of PortAudio's official C++ interface.
 * 
 * @author Charles Van West
 * @date 2016-07-16
 */

static_assert(sizeof(float) == 4, "float is the wrong size");

#include <portaudiocpp/PortAudioCpp.hxx>
#include <iostream>

constexpr const int SECONDS = 10; /* seconds to run */

constexpr const int CHANNELS = 1;
constexpr const portaudio::SampleDataFormat SAMPLE_FORMAT
        = portaudio::SampleDataFormat::FLOAT32;
using Sample_t = float;

constexpr const int SAMPLE_RATE = 44100;
constexpr const int FRAMES_PER_BUFFER = 32;

/**
 * Callback functor for the callback version of this program.
 * 
 * @version 0
 */
class Test_Callback : public portaudio::CallbackInterface {
public:
    Test_Callback() = default;
    ~Test_Callback() = default;
    
    int paCallbackFun(const void* input_raw,
                      void* output_raw,
                      unsigned long int input_frames,
                      const PaStreamCallbackTimeInfo* time_info,
                      PaStreamCallbackFlags status_flags) {
        /* convert the void*s into reasonable buffers */
        const Sample_t* input  = static_cast<const float*>(input_raw);
              Sample_t* output = static_cast<      float*>(output_raw);
        
        /* process the lesser number of frames (to prevent segfaults) */
        unsigned long int frames = FRAMES_PER_BUFFER < input_frames
                                 ? FRAMES_PER_BUFFER
                                 : input_frames;
        
        /* read the input into the output */
        for (int i = 0; i < frames; ++i) {
            output[i] = input[i];
        }
        
        return 0;
    }
private:
    Sample_t buffer[FRAMES_PER_BUFFER];
};

int main(int argc, char** argv) {
    portaudio::AutoSystem system_raii_handle; /* ensures the system is
                                                 terminated on exit */
    /* get system instance */
    portaudio::System& system = portaudio::System::instance();
    
    /* get default devices */
    portaudio::Device& input  = system.defaultInputDevice();
    portaudio::Device& output = system.defaultOutputDevice();
    
    /* set up the stream parameters */
    portaudio::DirectionSpecificStreamParameters
            input_parameters(input,
                             CHANNELS,
                             SAMPLE_FORMAT,
                             true, /* interleaved (segfaults if not set) */
                             input.defaultHighInputLatency(),
                             nullptr); /* nothing host-specific */
    portaudio::DirectionSpecificStreamParameters
            output_parameters(output,
                              CHANNELS,
                              SAMPLE_FORMAT,
                              true, /* interleaved (segfaults if not set) */
                              output.defaultHighOutputLatency(),
                              nullptr); /* nothing host-specific */
    portaudio::StreamParameters parameters(input_parameters,
                                           output_parameters,
                                           SAMPLE_RATE,
                                           FRAMES_PER_BUFFER,
                                           paClipOff);
    
    if (argc == 1) {
        /* open a blocking stream */
        portaudio::BlockingStream stream(parameters);
        stream.start();
        
        /* read and write some data */
        Sample_t buffer[FRAMES_PER_BUFFER];
        for (   int n = 0;
                n < SECONDS * SAMPLE_RATE / FRAMES_PER_BUFFER;
                ++n) {
            try {
                stream.read(buffer, FRAMES_PER_BUFFER);
                stream.write(buffer, FRAMES_PER_BUFFER);
            } catch (portaudio::PaException& exc) {
                std::cerr << "portaudio::PaException: " << exc.what() << std::endl;
            }
        }
        
        stream.stop();
    } else {
        /* open a callback stream */
        Test_Callback callback;
        portaudio::InterfaceCallbackStream stream(parameters, callback);
        stream.start();
        system.sleep(SECONDS * 1000);
        stream.stop();
    }
    
    return 0;
}
