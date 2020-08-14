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

typedef double(*easingFunction)(double);

easingFunction getEasingFunction( easing_functions function );

