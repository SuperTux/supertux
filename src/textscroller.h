#ifndef __TEXTSCROLLER_H__
#define __TEXTSCROLLER_H__

#include <vector>
#include <string>

namespace SuperTux {
class DrawingContext;
}

using namespace SuperTux;

/** This class is displaying a box with information text inside the game
 */
class InfoBox
{
public:
  InfoBox(const std::string& text);
  ~InfoBox();

  void draw(DrawingContext& context);
  void scrolldown();
  void scrollup();
  void pagedown();
  void pageup();
  
private:
  size_t firstline;
  std::vector<std::string> lines;
};

/** Reads a text file (using LispReader, so it as to be in its formatting)
 * and scroll it over the screen
 * (this call blocks until all text scrolled through or the user aborted the
 *  textscrolling)
 */
void display_text_file(const std::string& file);

#endif

