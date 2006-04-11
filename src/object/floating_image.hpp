#ifndef __FLOATING_IMAGE_H__
#define __FLOATING_IMAGE_H__

#include "game_object.hpp"
#include "math/vector.hpp"
#include "anchor_point.hpp"
#include <memory>

class Sprite;

class FloatingImage : public GameObject
{
public:
  FloatingImage(const std::string& sprite);
  virtual ~FloatingImage();

  void set_layer(int layer) {
    this->layer = layer;
  }
  
  int get_layer() const {
    return layer;
  }

  void set_pos(const Vector& pos) {
    this->pos = pos;
  }
  const Vector& get_pos() const {
    return pos;
  }
  
  void set_anchor_point(AnchorPoint anchor) {
    this->anchor = anchor;
  }
  AnchorPoint get_anchor_point() const {
    return anchor;
  }

  void set_visible(bool visible) {
    this->visible = visible;
  }
  bool get_visible() const {
    return visible;
  }

  void update(float elapsed_time);
  void draw(DrawingContext& context);

private:
  std::auto_ptr<Sprite> sprite;
  int layer;
  bool visible;
  AnchorPoint anchor;
  Vector pos;
};

#endif

