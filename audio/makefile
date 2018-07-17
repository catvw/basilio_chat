OBJECTS = core_audio.o

core_audio.o: core_audio.c++ core_audio.h++
	c++ -c -o core_audio.o core_audio.c++

audio_test: audio_test.c++ core_audio.o
	c++ -lportaudio -o audio_test audio_test.c++ core_audio.o

audio_test_pa: audio_test_pa.c++
	c++ -lportaudio -lportaudiocpp -o audio_test_pa audio_test_pa.c++

.PHONY: clean
clean:
	-rm audio_test audio_test_pa $(OBJECTS)
