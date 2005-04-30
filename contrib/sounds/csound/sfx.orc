
; supertux - various sound effect orchestra
;
;  note - The sounds can't be looped right away, an external program needs 
;         to be used to make them loop
;
;        Bastiaan Zapf - freenode.net basti_ 

sr = 44100
kr = 44100
ksmps = 1
nchnls = 1

; lava sound

instr 1
  ; Generate a random between 0 and 1.
  ; krange = 1

	kocrck init 0

	a1 exprand 10             ; exponential noise as a base

	kncrck exprand 1          ; get new crackle raw
	kcrck = (kocrck-kncrck)/2 ; dc correct crackle
	kocrck = kncrck           ; remember old crackle

	a1    pareq   a1, p4, 30    , 1 , 1 ; growl
	a1    pareq   a1, p5, 0     , 1 , 2 ; high stop
	a1    pareq   a1, p6, kncrck, p7, 0 ; "crackling" of various intensity

	a1    pareq   a1, 10, 0     , 1 , 1 ; thunder stop (dc correction)

	aout  =       a1*750 ; adjust your volume here

	out aout 

endin

; rain / water - this is quite similar to lava, just "crackles" more

instr 2

	a1 exprand 10

	kocrck init 0
	kncrck exprand 1 ; get new crackle
	k2crck = kncrck
 	kncrck = kncrck -1
	kncrck = kncrck^p8    ; few high values, many low values - how sparse
                              ; the highs are is controlled by the power (p8).
	kcrck = (kocrck-kncrck)/2 ; dc correct crackle
	kocrck = kncrck           ; remember old crackle

	a1    pareq   a1, p4,p5, 1, 1 ; growl
	a1    pareq   a1, p6  ,1+kncrck*p7 ,1, 0 ; "crackling"
	a1    pareq   a1, p6/2,1+k2crck*p7 ,1, 0 ; "crackling"

	a1    pareq   a1,  10, 0, 1, 1 ; thunder stop (dc correction)
	a1    pareq   a1, 4000, 0, 1, 2 ; extreme high stop

	out a1*700 ; adjust your volume here
endin
        

instr 3
	ifqc = cpspch(p4)

	kamp linseg 0,p3/4,1,p3/2,1,p3/4,0

	kff  init  1

	aout 	oscil3 p5,ifqc*kff,1  ; make a sound
	aout    butterhp aout,500,0.5 ; ear protection ;)

	kff = kff*p6 ; frequency exponential rise
	out aout*kamp

endin        