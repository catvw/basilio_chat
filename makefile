.PHONY: basilio_chat
basilio_chat:
	$(MAKE) -C terminal
	$(MAKE) -C ../socket
	$(MAKE) -C audio
	$(MAKE) -f basilio_chat.mk

.PHONY: clean
clean:
	-$(MAKE) -C terminal clean
	-$(MAKE) -C ../socket clean
	-$(MAKE) -C audio clean
	-$(MAKE) -f basilio_chat.mk clean
