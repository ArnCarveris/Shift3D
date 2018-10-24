#include <candle.h>
#include <components/node.h>
#include <components/name.h>
#include "state.h"
#include "side.h"
#include <stdlib.h>
#include <string.h>

c_state_t *c_state_new(const char *filename)
{
	c_state_t *self = component_new("state");

	strcpy(self->file, filename);

	if(!filename[0]) return self;

	self->scene = entity_new(c_name_new("scene"), c_node_new());

	candle_run(self->scene, filename);

	self->grid = c_node_get_by_name(c_node(&self->scene), ref("grid"));

	return self;
}

REG()
{
	ct_new("state", sizeof(c_state_t), NULL, NULL, 0);
}