// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.
// Algorithm was taken from here: http://stackoverflow.com/a/20039017/624766
// http://www.drdobbs.com/database/the-maximal-rectangle-problem/184410529

#ifndef __FIND_RECTS_HPP__
#define __FIND_RECTS_HPP__

#include <climits>

namespace FindRects {

	/** Input aray type, typically the smallest integral type */
	typedef unsigned char InputType;
	/** Output array type, which consists of the width and height of any given rectangle at this point */
	typedef unsigned char OutputType;
	enum { OUTPUT_MAX_LENGTH = UCHAR_MAX };

	/** Split an area into rectangles, trying to cover as much area as possible
		@param minLength: minimal length of the rectangle side, can be 1 to count every element in the array
		@param output: output array of width x height * 2, where each two elements are width/height of a rectangle's top-left corner, or 0 if the area is inside a rectangle
		@return total area size of all rectangles, covered by rectangles with side length = maxLength or bigger,
	*/
	long long findAll (const InputType* input, int width, int height, int minLength, OutputType* output);

}

#endif
