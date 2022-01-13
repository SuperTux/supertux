// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.
// Algorithm was taken from here: http://stackoverflow.com/a/20039017/624766
// http://www.drdobbs.com/database/the-maximal-rectangle-problem/184410529

#include <stdlib.h>
#include <vector>
#include <utility>
#include <algorithm>

#include "find_rects.hpp"

namespace FindRects {

struct Pair {
	int x;
	int y;
	Pair(int a, int b): x(a), y(b) {}
};

/** A rectangle, (x,y) is the upper-left corner, coordinates start at (0,0)
	TODO: replace it with Rect from rect.hpp */
struct Rect {
	int x;
	int y;
	int w;
	int h;
	Rect(int X, int Y, int W, int H): x(X), y(Y), w(W), h(H) {}
};

static void update_cache(std::vector<int> *c, int n, int M, int rowWidth, const InputType* input) {
	for (int m = 0; m != M; ++m) {
		if (input[n * rowWidth + m] != 0) {
			(*c)[m]++;
		} else {
			(*c)[m] = 0;
		}
	}
}

static Rect findBiggest(const InputType* input, int M, int N, int rowWidth) {

	Pair best_ll(0, 0); /* Lower-left corner */
	Pair best_ur(-1, -1); /* Upper-right corner */
	int best_area = 0;
	int best_perimeter = 0;

	std::vector<int> c(M + 1, 0); /* Cache */
	std::vector<Pair> s; /* Stack */
	s.reserve(M + 1);

	int m, n;

	/* Main algorithm: */
	for (n = 0; n != N; ++n) {
		int open_width = 0;
		update_cache(&c, n, M, rowWidth, input);
		for (m = 0; m != M + 1; ++m) {
			if (c[m] > open_width) { /* Open new rectangle? */
				s.push_back(Pair(m, open_width));
				open_width = c[m];
			} else if (c[m] < open_width) { /* Close rectangle(s)? */
				int m0, w0, area, perimeter;
				do {
					m0 = s.back().x;
					w0 = s.back().y;
					s.pop_back();
					area = open_width * (m - m0);
					perimeter = open_width + (m - m0);
					/* If the area is the same, prefer squares over long narrow rectangles,
						it finds more rectangles this way when calling findAll() with minLength == 2 or more */
					if (area > best_area || (area == best_area && perimeter < best_perimeter)) {
						best_area = area;
						best_perimeter = perimeter;
						best_ll.x = m0;
						best_ll.y = n;
						best_ur.x = m - 1;
						best_ur.y = n - open_width + 1;
					}
					open_width = w0;
				} while (c[m] < open_width);
				open_width = c[m];
				if (open_width != 0) {
					s.push_back(Pair(m0, w0));
				}
			}
		}
	}
	return Rect(best_ll.x, std::max(0, best_ur.y), 1 + best_ur.x - best_ll.x, 1 + best_ll.y - best_ur.y);
}

//#define CHECK_BOTH_WAYS 1 /* This will make the algorithm terribly slow, with a factorial complexity */

/** Find biggest rectangle, then recursively search area to the left/right and to the top/bottom of that rectangle
	for smaller rectangles, and choose the one that covers biggest area.
	@return biggest area size, covered by rectangles with side length = maxLength or bigger,
	@param search: limit searching for the following area
	@param output: may be nullptr, then the function will only return the area size
*/

static long long findRectsInArea (const InputType* input, int rowWidth, int minLength, OutputType* output, Rect search) {
	if (search.w < minLength || search.h < minLength) {
		return 0; // We reached a size limit
	}
	Rect biggest = findBiggest(input + search.y * rowWidth + search.x, search.w, search.h, rowWidth);

	if (biggest.w < minLength || biggest.h < minLength) {
		return 0; // No rectangles here
	}
	biggest.x += search.x;
	biggest.y += search.y;
	if (biggest.w > OUTPUT_MAX_LENGTH) {
		biggest.w = OUTPUT_MAX_LENGTH;
	}
	if (biggest.h > OUTPUT_MAX_LENGTH) {
		biggest.h = OUTPUT_MAX_LENGTH;
	}
	/* We got two choices to split remaining area into four rectangular regions, where (B) is the biggest rectangle:
		****|***|***				************
		****|***|***				************
		****BBBBB***				----BBBBB---
		****BBBBB***		vs		****BBBBB***
		****BBBBB***				----BBBBB---
		****|***|***				************
		We are not filling the output array in the first recursive call, it's just for determining the resulting area size
	*/

	if (output != nullptr) {
		for (int y = biggest.y; y < biggest.y + biggest.h; y++) {
			for (int x = biggest.x; x < biggest.x + biggest.w; x++) {
				output[(y * rowWidth + x) * 2] = 0;
				output[(y * rowWidth + x) * 2 + 1] = 0;
			}
		}
		output[(biggest.y * rowWidth + biggest.x) * 2] = static_cast<OutputType>(biggest.w);
		output[(biggest.y * rowWidth + biggest.x) * 2 + 1] = static_cast<OutputType>(biggest.h);
	}

#ifdef CHECK_BOTH_WAYS
	long long splitHorizArea =
		findRectsInArea(input, rowWidth, minLength, nullptr,
			Rect(search.x, search.y, biggest.x - search.x, search.h)) +
		findRectsInArea(input, rowWidth, minLength, nullptr,
			Rect(biggest.x + biggest.w, search.y, search.x + search.w - biggest.x - biggest.w, search.h)) +
		findRectsInArea(input, rowWidth, minLength, nullptr,
			Rect(biggest.x, search.y, biggest.w, biggest.y - search.y)) +
		findRectsInArea(input, rowWidth, minLength, nullptr,
			Rect(biggest.x, biggest.y + biggest.h, biggest.w, search.y + search.h - biggest.y - biggest.h));

	long long splitVertArea =
		findRectsInArea(input, rowWidth, minLength, nullptr,
			Rect(search.x, search.y, search.w, biggest.y - search.y)) +
		findRectsInArea(input, rowWidth, minLength, nullptr,
			Rect(search.x, biggest.y + biggest.h, search.w, search.y + search.h - biggest.y - biggest.h)) +
		findRectsInArea(input, rowWidth, minLength, nullptr,
			Rect(search.x, biggest.y, biggest.x - search.x, biggest.h)) +
		findRectsInArea(input, rowWidth, minLength, nullptr,
			Rect(biggest.x + biggest.w, biggest.y, search.x + search.w - biggest.x - biggest.w, biggest.h));

	/* Inefficiently perform the recursive call again, this time with non-nullptr output array */
	if (splitHorizArea > splitVertArea) {
		if (output != nullptr) {
#endif
			return static_cast<long long>(biggest.w) * biggest.h +
			findRectsInArea(input, rowWidth, minLength, output,
				Rect(search.x, search.y, biggest.x - search.x, search.h)) +
			findRectsInArea(input, rowWidth, minLength, output,
				Rect(biggest.x + biggest.w, search.y, search.x + search.w - biggest.x - biggest.w, search.h)) +
			findRectsInArea(input, rowWidth, minLength, output,
				Rect(biggest.x, search.y, biggest.w, biggest.y - search.y)) +
			findRectsInArea(input, rowWidth, minLength, output,
				Rect(biggest.x, biggest.y + biggest.h, biggest.w, search.y + search.h - biggest.y - biggest.h));
#ifdef CHECK_BOTH_WAYS
		}
		return splitHorizArea + static_cast<long long>(biggest.w) * biggest.h;
	} else {
		if (output != nullptr) {
			findRectsInArea(input, rowWidth, minLength, output,
				Rect(search.x, search.y, search.w, biggest.y - search.y));
			findRectsInArea(input, rowWidth, minLength, output,
				Rect(search.x, biggest.y + biggest.h, search.w, search.y + search.h - biggest.y - biggest.h));
			findRectsInArea(input, rowWidth, minLength, output,
				Rect(search.x, biggest.y, biggest.x - search.x, biggest.h));
			findRectsInArea(input, rowWidth, minLength, output,
				Rect(biggest.x + biggest.w, biggest.y, search.x + search.w - biggest.x - biggest.w, biggest.h));
		}
		return splitVertArea + static_cast<long long>(biggest.w) * biggest.h;
	}
#endif
}

long long findAll (const InputType* input, int width, int height, int minLength, OutputType* output) {
	return findRectsInArea(input, width, minLength, output, Rect(0, 0, width, height));
}

} /* namespace */
