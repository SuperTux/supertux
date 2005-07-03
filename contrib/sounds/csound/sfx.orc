
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
	a1    pareq   a1, p6/3,1+k2crck*p7 ,1, 0 ; "crackling"

	a1    pareq   a1, 10, 0, 0.7, 1 ; thunder stop (dc correction)
	a1    pareq   a1, p9, 0, 0.7, 2 ; extreme high stop

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

instr 4

	ifqc= cpspch(p4)
	ihold

	kamp linseg 0,p3/8,1,p3*6/8,1,p3/8,0
	kfenv expseg 0.001,p3/8,3,p3,1,p3*1,0.001

	asrc oscili p5/4,ifqc,1
	aout lowres asrc,ifqc*kfenv/2,2
	aphs phaser2 aout, ifqc*(kfenv+1), 0.89,20, 2, 2, 0.6

	aout = aout - aphs

	aecho reverb aout,0.01,0.1
	aout=aout+aecho*0.3

	aout balance aout,asrc

	aout butterhp aout,1000,0


	out aout*kamp

endin


instr 5 ; Wind

	ifqc= cpspch(p4)
	
	kfmanip oscil3 1,1/p3,1
	kvol    linseg 0,0.1*p3,1,0.8*p3,1,0.1*p3,0
	aout	oscil3 p5,ifqc*kfmanip,2
	aout 	butterhp aout,2000,0.7
	aout 	butterlp aout,10000,0.7
	aout    phaser2 aout,ifqc*kfmanip,0.7,1,0.2,0.5
	out aout*kvol*(kfmanip/2.5)
endin

instr 6 
	aout1 dripwater 1,0.1,50,1.98,300,350,430
	aout2 dripwater 1,0.1,50,1.98,400,440,580
	aout3 dripwater 1,0.1,50,1.98,500,530,600
	aout4 dripwater 1,0.1,200,1.96,300,350,430
	aout5 dripwater 1,0.1,200,1.96,400,440,580
	aout6 dripwater 1,0.1,200,1.96,500,530,600

	aout =aout1+aout2+aout3+aout4+aout5+aout6
      	aout2 oscili 1,100+aout/10000,1
	aout2 =aout2*aout/1000+aout/1000
	aout = aout2
	aout butterlp aout,5000,0.4
    	out aout
endin