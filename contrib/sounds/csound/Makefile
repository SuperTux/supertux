
CSOUND = /usr/bin/csound

sounds: bubble.wav rain.wav lava.wav waterfall.wav 

%.wav: %.sco sfx.orc 
	$(CSOUND) sfx.orc $< -W -o $@ -g

clean: 
	rm *.wav
	rm *~

