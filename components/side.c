#include "side.h"
#include <stdlib.h>


void c_side_init(c_side_t *self)
{
	self->side = 2;
}

c_side_t *c_side_new(int side)
{
	c_side_t *self = component_new("side");

	self->side = side;
	return self;
}

REG()
{
	ct_new("side", sizeof(c_side_t), c_side_init, NULL, 0);
}


