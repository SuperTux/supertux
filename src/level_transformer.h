#ifndef __LEVEL_TRANSFORMER_H__
#define __LEVEL_TRANSFORMER_H__

class Level;
class Sector;

/**
 * This class is an abstract interface for algorithms that transform levels in
 * some way before they are played.
 */
class LevelTransformer
{
public:
  virtual ~LevelTransformer();
  
  /** transform a complete Level, the standard implementation just calls
   * transformSector on each sector in the level. 
   */
  virtual void transform(Level* level);

  virtual void transform_sector(Sector* sector) = 0;
};

#endif

