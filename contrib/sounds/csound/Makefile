
CSOUND = /usr/bin/csound
SOX = /usr/bin/sox

sounds: bubble.wav rain.wav lava.wav waterfall.wav flower.wav wind.wav

%.wav: %.sco sfx.orc 
	$(CSOUND) sfx.orc $< -W -o $@.1 -g
	$(SOX) -r 22100 -s $@.1 $@
	rm $@.1

clean: 
	rm *.wav
	rm *~

