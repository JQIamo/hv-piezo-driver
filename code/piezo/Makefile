
ARDUINOPATH = /Applications/Arduino.app/Contents/Java/

ARDUINOBUILDER = $(ARDUINOPATH)/arduino-builder

TOOLS = $(ARDUINOPATH)/hardware/tools/


SKETCH = $(wildcard *.ino)

upload: compile
	$(TOOLS)/teensy_post_compile -file=$(SKETCH) -path=$(CURDIR)/build -tools=$(TOOLS) -board=TEENSY31 -reboot


compile:
	mkdir -p build
	$(ARDUINOBUILDER) -compile -verbose -build-options-file build.options.json -build-path $(CURDIR)/build $(SKETCH)
	cp $(CURDIR)/build/$(SKETCH).hex $(CURDIR)/bin
	cp $(CURDIR)/build/$(SKETCH).elf $(CURDIR)/bin


clean:
	rm -rf build/
