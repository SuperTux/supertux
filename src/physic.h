//
// C++ Interface: physic
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_PHYSIC_H
#define SUPERTUX_PHYSIC_H

/** This is a very simplistic physics engine handling accelerated and constant
  * movement along with gravity.
  */
class Physic
{
public:
    Physic();
    ~Physic();

    /** resets all velocities and accelerations to 0 */
    void reset();

    /** sets velocity to a fixed value */
    void set_velocity(float vx, float vy);

    /** velocities invertion */
    void inverse_velocity_x();
    void inverse_velocity_y();

    float get_velocity_x();
    float get_velocity_y();
    
    /** sets acceleration applied to the object. (Note that gravity is
     * eventually added to the vertical acceleration)
     */
    void set_acceleration(float ax, float ay);

    float get_acceleration_x();
    float get_acceleration_y();

    /** enables or disables handling of gravity */
    void enable_gravity(bool gravity_enabled);

    /** applies the physical simulation to given x and y coordinates */
    void apply(float frame_ratio, float &x, float &y); 

private:
    /// horizontal and vertical acceleration
    float ax, ay;
    /// horizontal and vertical velocity
    float vx, vy;
    /// should we respect gravity in out calculations?
    bool gravity_enabled;
};

#endif /*SUPERTUX_PHYSIC_H*/
