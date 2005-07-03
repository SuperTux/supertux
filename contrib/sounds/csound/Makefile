
CSOUND = /usr/bin/csound
SOX = /usr/bin/sox

sounds: bubble.ogg rain.ogg lava.ogg waterfall.ogg flower.ogg wind.ogg splash.ogg

%.ogg: %.sco sfx.orc 
	$(CSOUND) sfx.orc $< -W -o $*.wav -g -s
	oggenc $*.wav
	rm $*.wav
        

clean: 
	rm *.ogg
	rm *.wav
	rm *~


