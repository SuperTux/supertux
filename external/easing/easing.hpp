//
//  easing.h
//
//  Copyright (c) 2011, Auerhaus Development, LLC
//
//  This program is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What The Fuck You Want
//  To Public License, Version 2, as published by Sam Hocevar. See
//  http://sam.zoy.org/wtfpl/COPYING for more details.
//

// DISCLAIMER : This file, as well as the linked C source file, have been
// modified for compatibility purposes.
//     Edited by Semphris for SuperTux, Aug 14, 2020
//

#ifndef EASING_H
#define EASING_H

extern "C" {

enum EasingMode
{
	EaseNone,
	EaseQuadIn,
	EaseQuadOut,
	EaseQuadInOut,
	EaseCubicIn,
	EaseCubicOut,
	EaseCubicInOut,
	EaseQuartIn,
	EaseQuartOut,
	EaseQuartInOut,
	EaseQuintIn,
	EaseQuintOut,
	EaseQuintInOut,
	EaseSineIn,
	EaseSineOut,
	EaseSineInOut,
	EaseCircularIn,
	EaseCircularOut,
	EaseCircularInOut,
	EaseExponentialIn,
	EaseExponentialOut,
	EaseExponentialInOut,
	EaseElasticIn,
	EaseElasticOut,
	EaseElasticInOut,
	EaseBackIn,
	EaseBackOut,
	EaseBackInOut,
	EaseBounceIn,
	EaseBounceOut,
	EaseBounceInOut
};

float LinearInterpolation(float p);
float QuadraticEaseIn(float p);
float QuadraticEaseOut(float p);
float QuadraticEaseInOut(float p);
float CubicEaseIn(float p);
float CubicEaseOut(float p);
float CubicEaseInOut(float p);
float QuarticEaseIn(float p);
float QuarticEaseOut(float p);
float QuarticEaseInOut(float p);
float QuinticEaseIn(float p);
float QuinticEaseOut(float p);
float QuinticEaseInOut(float p);
float SineEaseIn(float p);
float SineEaseOut(float p);
float SineEaseInOut(float p);
float CircularEaseIn(float p);
float CircularEaseOut(float p);
float CircularEaseInOut(float p);
float ExponentialEaseIn(float p);
float ExponentialEaseOut(float p);
float ExponentialEaseInOut(float p);
float ElasticEaseIn(float p);
float ElasticEaseOut(float p);
float ElasticEaseInOut(float p);
float BackEaseIn(float p);
float BackEaseOut(float p);
float BackEaseInOut(float p);
float BounceEaseIn(float p);
float BounceEaseOut(float p);
float BounceEaseInOut(float p);

typedef float(*easing)(float);
easing getEasingByName(EasingMode ease_type);
EasingMode EasingMode_from_string(std::string ease_name);
std::string getEasingName(EasingMode ease_type);

}

#endif
