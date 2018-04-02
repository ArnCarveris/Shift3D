#include "charlook.h"
#include "side.h"
#include <systems/window.h>
#include <components/spacial.h>
#include <components/force.h>
#include <components/node.h>
#include <mouse.h>
#include <candle.h>
#include <math.h>
#include <stdlib.h>


void c_charlook_init(c_charlook_t *self)
{
	self->win_min_side = 1080;
}

c_charlook_t *c_charlook_new(entity_t x, float sensitivity)
{
	c_charlook_t *self = component_new("charlook");

	self->sensitivity = sensitivity;
	candle_grab_mouse(candle, c_entity(self), 0);

	self->x = x;

	return self;
}

static int c_charlook_window_resize(c_charlook_t *self,
		window_resize_data *event)
{
	self->win_min_side = (event->width < event->height) ?
		event->width : event->height;
	return 1;
}

void c_charlook_rotate(c_charlook_t *self, float angle)
{
	c_spacial_t *sc = c_spacial(self);

	float cosy = cosf(angle);
	float siny = sinf(angle);
	self->zrot += angle;

	float front_angle = self->xrot + M_PI / 2;
	vec3_t front = (vec3(cos(front_angle), 0, sin(front_angle)));

	c_spacial_lock(sc);
	c_spacial_set_pos(sc, vec3_rotate(sc->pos, front, cosy, siny));
	c_spacial_unlock(sc);
	c_spacial_rotate_Z(sc, angle);
}

int c_charlook_update(c_charlook_t *self, float *dt)
{
	float dif;
	float targ = self->side ? M_PI : 0;
	if(fabs(dif = targ - self->zrot) > 0.01)
	{
		float inc = dif * 5 * (*dt);
		c_charlook_rotate(self, inc);
	}
	return 1;
}

void c_charlook_toggle_side(c_charlook_t *self)
{
	if(!self->side)
	{
		self->side = 1;
	}
	else
	{
		self->side = 0;

	}

}


int c_charlook_mouse_move(c_charlook_t *self, mouse_move_data *event)
{
	float frac = self->sensitivity / self->win_min_side;
	float inc_x = -event->sx * frac;
	float inc_y = -event->sy * frac;

	const float max_up = M_PI / 2.0 - 0.01;
	const float max_down = -M_PI / 2.0 + 0.01;


	c_spacial_t *sc = c_spacial(self);

	if(self->xrot > max_up && inc_y > 0) inc_y = 0;
	if(self->xrot < max_down && inc_y < 0) inc_y = 0;

	c_side_t *sidec = c_side(&candle->systems);
	if(!sidec) return 1;
	int side = sidec->side;
	inc_x = side ? -inc_x : inc_x;

	self->yrot += inc_x;
	self->xrot += inc_y;

	c_spacial_rotate_X(sc, inc_y);

	sc = c_spacial(&self->x);
	float old_rot = sc->rot.z;

	c_spacial_rotate_Z(sc, -old_rot);
	c_spacial_rotate_Y(sc, inc_x);
	c_spacial_rotate_Z(sc, old_rot);




	return 1;
}

REG()
{
	ct_t *ct = ct_new("charlook", sizeof(c_charlook_t),
			(init_cb)c_charlook_init, 1, ref("node"));

	ct_listener(ct, WORLD, sig("mouse_move"), c_charlook_mouse_move);

	ct_listener(ct, WORLD, sig("window_resize"), c_charlook_window_resize);

	ct_listener(ct, WORLD, sig("world_update"), c_charlook_update);
}

