OBJECTS = core_audio.o

core_audio.o: core_audio.c++ core_audio.h++
	c++ -I/Users/Charlie/.cpath/portaudio -c \
	-o core_audio.o core_audio.c++

audio_test: audio_test.c++ core_audio.o
	c++ -I/Users/Charlie/.cpath/portaudio \
	-L/Users/Charlie/.cpath/portaudio/lib -lportaudio \
	-o audio_test audio_test.c++ core_audio.o

.PHONY: clean
clean:
	-rm audio_test $(OBJECTS)
