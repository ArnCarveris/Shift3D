#include <candle.h>
#include "movable.h"
#include "level.h"
#include "side.h"
#include "grid.h"
#include <stdlib.h>
#include <components/spacial.h>


void c_movable_init(c_movable_t *self) { }

c_movable_t *c_movable_new(int value)
{
	c_movable_t *self = component_new("c_movable");
	self->value = value;

	return self;
}

void push_at(int x, int y, int z, int value, vec3_t from)
{
	int i, p;
	c_level_t *level = c_level(&candle->systems);
	c_grid_t *gc = c_grid(&level->grid);

	ct_t *movables = ecm_get(ref("c_movable"));
	vec3_t pos = vec3(x, y, z);

	vec3_t dif = vec3_sub(from, pos);
	if(fabs(dif.x) > fabs(dif.z))
	{
		if(dif.x < 0) dif.x = -1;
		if(dif.x > 0) dif.x = 1;
		dif.z = 0;
	}
	else
	{
		if(dif.z < 0) dif.z = -1;
		if(dif.z > 0) dif.z = 1;
		dif.x = 0;
	}

	int air = !(value & 1);
	int next = c_grid_get(gc,
			round(x - dif.x), y, round(z - dif.z));

	if((next & 1) != air || next == -1)
	{
		return;
	}
	c_grid_set(gc, x, y, z, air);

	for(p = 0; p < movables->pages_size; p++)
	for(i = 0; i < movables->pages[p].components_size; i++)
	{
		c_movable_t *m = (c_movable_t*)ct_get_at(movables, p, i);
		if(m->value == value) if(vec3_dist(c_spacial(m)->pos, pos) < 0.4)
		{
			m->mx = -dif.x;
			m->my = -dif.y;
			m->mz = -dif.z;
			m->moving = 1;
			m->x = x;
			m->y = y;
			m->z = z;
		}
	}
}

static int c_movable_update(c_movable_t *self, float *dt)
{
	if(!self->moving) return 1;
	float inc = (*dt) * 6;
	c_spacial_t *sc = c_spacial(self);

	if(self->mx > 0)	
	{
		inc = fmin(inc, self->mx);
		sc->pos.x += inc;
		self->mx -= inc;
		c_spacial_set_pos(sc, sc->pos);
	}
	else if(self->mx < 0)	
	{
		inc = fmin(inc, -self->mx);
		sc->pos.x -= inc;
		self->mx += inc;
		c_spacial_set_pos(sc, sc->pos);
	}
	else if(self->mz > 0)	
	{
		inc = fmin(inc, self->mz);
		sc->pos.z += inc;
		self->mz -= inc;
		c_spacial_set_pos(sc, sc->pos);
	}
	else if(self->mz < 0)	
	{
		inc = fmin(inc, -self->mz);
		sc->pos.z -= inc;
		self->mz += inc;
		c_spacial_set_pos(sc, sc->pos);
	}
	else
	{
		c_level_t *level = c_level(&candle->systems);
		c_grid_t *gc = c_grid(&level->grid);

		int side = c_side(self)->side;
		int dir = (side?side:-1);
		vec3_t rnd = vec3_round(sc->pos);

		int ground = c_grid_get(gc, rnd.x, rnd.y + dir, rnd.z);

		if((ground&1) != side || ground == -1)
		{
			c_spacial_set_pos(sc, rnd);
			c_grid_set(gc, sc->pos.x, sc->pos.y, sc->pos.z, 2 | (!side));
			self->moving = 0;
			self->sy = 0;
			entity_signal(c_entity(self), sig("grid_update"), NULL);
		}
		else
		{
			self->sy += dir * 0.4 * *dt;
			sc->pos.y += self->sy;
			c_spacial_set_pos(sc, sc->pos);
		}
	}

	return 1;
}

REG()
{
	ct_t *ct = ct_new("c_movable", sizeof(c_movable_t),
			(init_cb)c_movable_init, 0);

	ct_listener(ct, WORLD, sig("world_update"), c_movable_update);
}


