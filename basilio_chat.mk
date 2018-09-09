OBJECTS = basilio_chat.o packet.o

OS_NAME := $(shell uname -s)

ifeq ($(OS_NAME), Linux) # on linux
	LIBRARY_LINK = -lpthread -lpulse -lpulse-simple
else ifeq ($(OS_NAME), Darwin) #on a mac
		LIBRARY_LINK = -lportaudio -largp
endif

basilio_chat: main.c++ basilio_chat.o packet.o terminal/terminal.o ../socket/socket.o audio/core_audio.o
	c++ $(LIBRARY_LINK) \
	-o basilio_chat main.c++ basilio_chat.o packet.o terminal/terminal.o \
	../socket/socket.o audio/core_audio.o

basilio_chat.o: basilio_chat.c++ basilio_chat.h++
	c++ -c -o basilio_chat.o basilio_chat.c++

packet.o: packet.c++ packet.h++
	c++ -c -o packet.o packet.c++

.PHONY: clean
clean:
	-rm basilio_chat $(OBJECTS)
