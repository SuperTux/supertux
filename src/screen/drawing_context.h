#ifndef __DRAWINGCONTEXT_H__
#define __DRAWINGCONTEXT_H__

#include <vector>
#include <string>
#include "vector.h"
#include <SDL.h>

class Surface;
class Font;

// some constants for predefined layer values
enum {
  LAYER_BACKGROUND0 = -300,
  LAYER_BACKGROUND1 = -200,
  LAYER_BACKGROUNDTILES = -100,
  LAYER_TILES = 0,
  LAYER_OBJECTS = 100,
  LAYER_FOREGROUNDTILES = 200,
  LAYER_FOREGROUND0 = 300,
  LAYER_FOREGROUND1 = 400
};

class Color
{
public:
  Color() 
    : red(0), green(0), blue(0), alpha(0)
  {}
  
  Color(Uint8 red_, Uint8 green_, Uint8 blue_, Uint8 alpha_ = 0)
    : red(red_), green(green_), blue(blue_), alpha(alpha_)
  {}

  Color(const Color& o)
    : red(o.red), green(o.green), blue(o.blue), alpha(o.alpha)
  { }

  Uint8 red, green, blue, alpha;
};

/**
 * This class provides functions for drawing things on screen. It also
 * maintains a stack of transforms that are applied to graphics.
 */
class DrawingContext
{
public:
  DrawingContext();
  ~DrawingContext();

  /** Adds a drawing request for a surface into the request list */
  void draw_surface(const Surface* surface, const Vector& position, int layer);
  /** Adds a drawing request for part of a surface */
  void draw_surface_part(const Surface* surface, const Vector& source,
      const Vector& size, const Vector& dest, int layer);
  /** draws a text */
  void draw_text(Font* font, const std::string& text, const Vector& position,
      int layer);
  /** draws aligned text */
  void draw_text_center(Font* font, const std::string& text,
      const Vector& position, int layer);
  /** draws a color gradient onto the whole screen */  
  void draw_gradient(Color from, Color to, int layer);
  /** fills a rectangle */
  void draw_filled_rect(const Vector& topleft, const Vector& downright,
          Color color, int layer);
  
  /** Processes all pending drawing requests and flushes the list */
  void do_drawing();

  const Vector& get_translation() const
  { return transform.translation; }
  void set_translation(const Vector& newtranslation)
  { transform.translation = newtranslation; }

  void push_transform();
  void pop_transform();

private:
  class Transform
  {
  public:
    Vector translation; // only translation for now...

    Vector apply(const Vector& v) const
    {
      return v - translation;
    }
  };

  /// the transform stack
  std::vector<Transform> transformstack;
  /// the currently active transform
  Transform transform;

  enum RequestType
  {
    SURFACE, SURFACE_PART, TEXT, GRADIENT, FILLRECT
  };

  struct SurfacePartRequest
  {
    const Surface* surface;
    Vector source, size;
  };

  struct TextRequest
  {
    Font* font;
    std::string text;
  };

  struct GradientRequest
  {
    Color top, bottom;
    Vector size;
  };

  struct FillRectRequest
  {
    Color color;
    Vector size;
  };

  struct DrawingRequest
  {
    int layer;

    RequestType type;
    Vector pos;

    void* request_data;

    bool operator<(const DrawingRequest& other) const
    {
      return layer < other.layer;
    }
  };

  void draw_surface_part(DrawingRequest& request);
  void draw_text(DrawingRequest& request);
  void draw_gradient(DrawingRequest& request);
  void draw_filled_rect(DrawingRequest& request);
  
  typedef std::vector<DrawingRequest> DrawingRequests;
  DrawingRequests drawingrequests;
};

#endif

