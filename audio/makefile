OBJECTS = core_audio.o

OS_NAME := $(shell uname -s)

ifeq ($(OS_NAME), Linux) # on linux
	AUDIO_MACRO = -D USE_PULSEAUDIO_FOR_SOUND
	AUDIO_LIBRARY = -lpulse -lpulse-simple
else ifeq ($(OS_NAME), Darwin) #on a mac
		AUDIO_MACRO = -D USE_PORTAUDIO_FOR_SOUND
		AUDIO_LIBRARY = -lportaudio
endif

core_audio.o: core_audio.c++ core_audio.h++
	c++ $(AUDIO_MACRO) -c -o core_audio.o core_audio.c++

audio_test: audio_test.c++ core_audio.o
	c++ $(AUDIO_LIBRARY) -o audio_test audio_test.c++ core_audio.o

audio_test_pa: audio_test_pa.c++
	c++ -lportaudio -lportaudiocpp -o audio_test_pa audio_test_pa.c++

.PHONY: clean
clean:
	-rm audio_test audio_test_pa $(OBJECTS)
