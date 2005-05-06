#ifndef __TEXT_H__
#define __TEXT_H__

namespace Scripting
{

class Text
{
public:
#ifndef SCRIPTING_API
  virtual ~Text()
  { }
#endif
  
  virtual void set_text(const std::string& text) = 0;
  virtual void set_font(const std::string& fontname) = 0;
  virtual void fade_in(float fadetime) = 0;
  virtual void fade_out(float fadetime) = 0;
  virtual void set_visible(bool visible) = 0;
};

}

#endif

