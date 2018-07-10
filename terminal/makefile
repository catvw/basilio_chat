OBJECTS = display.o input_reader.o terminal_manager.o

terminal.o: $(OBJECTS)
	ld -r -o terminal.o $(OBJECTS)

display.o: display.c++ ../sync_queue.t++ terminal_manager.h++
	c++ -I.. -c -o display.o display.c++

input_reader.o: input_reader.c++ ../sync_queue.t++ terminal_manager.h++
	c++ -I.. -c -o input_reader.o input_reader.c++

terminal_manager.o: terminal_manager.c++ ../sync_queue.t++ terminal_manager.h++
	c++ -I.. -c -o terminal_manager.o terminal_manager.c++

terminal_test: terminal_test.c++ terminal_manager.h++ ../sync_queue.t++ terminal.o
	c++ -I.. -o terminal_test terminal_test.c++ terminal.o

.PHONY: clean
clean:
	-rm terminal_test terminal.o $(OBJECTS)
