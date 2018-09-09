#include "core_audio.h++"

/* annoyingly need to do this */
template<int S, typename B>
vanwestco::Audio_Handle::Audio_Stream<S, B>::~Audio_Stream() { }

#ifdef USE_PORTAUDIO_FOR_SOUND

#include "portaudio_stream.t++"
vanwestco::Audio_Handle::Audio_Handle()
: stream(new PortAudio_Stream<sample_rate, Block_t>()) { }

#else
#ifdef USE_PULSEAUDIO_FOR_SOUND

#include "pulseaudio_stream.t++"
vanwestco::Audio_Handle::Audio_Handle()
: stream(new PulseAudio_Stream<sample_rate, Block_t>()) { }

#endif /* ~USE_PULSEAUDIO_FOR_SOUND */
#endif /* ~USE_PORTAUDIO_FOR_SOUND */
