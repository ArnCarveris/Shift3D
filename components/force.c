#include "force.h"
#include "velocity.h"


void c_force_init(c_force_t *self)
{
	self->force = vec3(0.0, 0.0, 0.0);
	self->active = 1;
}

c_force_t *c_force_new(float x, float y, float z, int active)
{
	c_force_t *self = component_new("force");

	self->active = active;
	self->force = vec3(x, y, z);

	return self;
}

REG()
{
	ct_new("force", sizeof(c_force_t), (init_cb)c_force_init, NULL, 0);
}

