#pragma once 


// WARNING : IF YOU CHANGE THEIR ORDER, MAKE SURE THEY FOLLOW THE SAME ORDER AS
// IN src/editor/node_markar.cpp, `ObjectSettings NodeMarker::get_settings()`
// must contain the EXACT SAME list in the EXACT SAME ORDER.

// FIXME: make it so there aren't two lists, if that's possible
enum easing_functions
{
	EaseNone,
	EaseInSine,
	EaseOutSine,
	EaseInOutSine,
	EaseInQuad,
	EaseOutQuad,
	EaseInOutQuad,
	EaseInCubic,
	EaseOutCubic,
	EaseInOutCubic,
	EaseInQuart,
	EaseOutQuart,
	EaseInOutQuart,
	EaseInQuint,
	EaseOutQuint,
	EaseInOutQuint,
	EaseInExpo,
	EaseOutExpo,
	EaseInOutExpo,
	EaseInCirc,
	EaseOutCirc,
	EaseInOutCirc,
	EaseInBack,
	EaseOutBack,
	EaseInOutBack,
	EaseInElastic,
	EaseOutElastic,
	EaseInOutElastic,
	EaseInBounce,
	EaseOutBounce,
	EaseInOutBounce
};

double easeNone(double);
double easeInSine(double);
double easeOutSine(double);
double easeInOutSine(double);
double easeInQuad(double);
double easeOutQuad(double);
double easeInOutQuad(double);
double easeInCubic(double);
double easeOutCubic(double);
double easeInOutCubic(double);
double easeInQuart(double);
double easeOutQuart(double);
double easeInOutQuart(double);
double easeInQuint(double);
double easeOutQuint(double);
double easeInOutQuint(double);
double easeInExpo(double);
double easeOutExpo(double);
double easeInOutExpo(double);
double easeInCirc(double);
double easeOutCirc(double);
double easeInOutCirc(double);
double easeInBack(double);
double easeOutBack(double);
double easeInOutBack(double);
double easeInElastic(double);
double easeOutElastic(double);
double easeInOutElastic(double);
double easeInBounce(double);
double easeOutBounce(double);
double easeInOutBounce(double);

typedef double(*easingFunction)(double);

easingFunction getEasingFunction( easing_functions function );

