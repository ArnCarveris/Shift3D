#include "cmds.h"
#include "components/side.h"
#include "components/grid.h"
#include "components/key.h"
#include "components/bridge.h"
#include "components/door.h"
#include "components/movable.h"
#include "components/state.h"
#include "components/character.h"
#include "components/charlook.h"
#include "components/side_follow.h"

#include <components/model.h>
#include <components/node.h>
#include <components/name.h>
#include <components/force.h>
#include <components/light.h>

#include <stdio.h>
#include <stdlib.h>

/* static int cmd_light_before_draw(c_light_t *self) */
/* { */
/* 	int side = c_side(self->super.entity)->side; */
/* 	return side == 2 || side == c_side(&candle->systems)->side; */
/* } */

static entity_t cmd_shift_grid(entity_t root, int argc, const char **argv)
{
	int mx, my, mz;
	int i;
	/* char c; */
	int l;
	entity_t entity;

	sscanf(argv[1], "%d", &mx);
	sscanf(argv[2], "%d", &my);
	sscanf(argv[3], "%d", &mz);

	entity = entity_new(c_name_new("grid"), c_grid_new(mx, my, mz));

	c_grid_t *grid = c_grid(&entity);

	l = mx * my * mz;

	for(i = 0; i < l; i++)
	{
		grid->map[i] = argv[4][i] - '0';
	}

	/* mesh_t *mesh = mesh_new(); */
	/* mesh_cube(mesh, 1, 1); */
	mesh_t *mesh = mesh_new();
	mesh_cuboid(mesh, 0.5,
			vec3(-0.5, -0.5, -0.5),
			vec3(0.5, 0.5, 0.5));

	int x, y, z;
	for(x = 0; x < grid->mx; x++)
		for(y = 0; y < grid->my; y++)
			for(z = 0; z < grid->mz; z++)
	{
		int val = c_grid_get(grid, x, y, z);
		if(val & 0x2)
		{
			entity_t box = entity_new(c_movable_new(val), c_side_new(!(val&1), 1),
					c_node_new(),
					c_model_new(mesh, sauces("stone3.mat"), 1, 1));
			/* c_model(&box)->before_draw = (before_draw_cb)cmd_model_before_draw; */
			c_spacial_set_pos(c_spacial(&box), vec3(x, y, z));
			/* c_node_add(c_node(&root), 1, box); */
		}
	}

	/* getc(fd); */

	entity_t scene = c_state(&SYS)->scene;

	entity_signal(entity, sig("grid_update"), NULL, NULL);
	c_node_add(c_node(&scene), 1, entity);

	return entity;
}

static entity_t cmd_key(entity_t root, int argc, const char **argv)
{
	float x, y, z;
	int rotX, rotY, rotZ;
	int id, side;

	sscanf(argv[1], "%f", &x);
	sscanf(argv[2], "%f", &y);
	sscanf(argv[3], "%f", &z);
	sscanf(argv[4], "%d", &rotX);
	sscanf(argv[5], "%d", &rotY);
	sscanf(argv[6], "%d", &rotZ);
	sscanf(argv[7], "%d", &id);

	entity_t scene = c_state(&SYS)->scene;

	entity_t grid = c_node_get_by_name(c_node(&scene), ref("grid"));
	side = c_grid_get(c_grid(&grid), x, y, z) & 1;

	entity_t key = entity_new(c_name_new("key"),
			c_node_new(),
			c_model_new(sauces("key.obj"), sauces("key.mat"), 1, 1),
			c_side_new(side, 0),
			c_key_new(rotX, rotY, rotZ, id));

	/* sauces_mat("key")->transparency.color = vec4(1.0f, 1.0f, 1.0f, 0.4f); */
	/* c_model(&key)->before_draw = (before_draw_cb)cmd_model_before_draw; */

	c_spacial_set_pos(c_spacial(&key), vec3(x, y, z));

	c_node_add(c_node(&scene), 1, key);
	return key;
}

static entity_t cmd_spawn(entity_t root, int argc, const char **argv)
{
	float x, y, z;
	int dir, side;

	sscanf(argv[1], "%f", &x);
	sscanf(argv[2], "%f", &y);
	sscanf(argv[3], "%f", &z);
	sscanf(argv[4], "%d", &dir);

	entity_t scene = c_state(&SYS)->scene;

	entity_t grid = c_node_get_by_name(c_node(&scene), ref("grid"));
	side = c_grid_get(c_grid(&grid), x, y, z) & 1;

	entity_t spawn = entity_new(c_node_new(),
			c_side_new(side, 1),
			c_spacial_new(),
			c_name_new("spawn"));
	c_state(&SYS)->spawn = spawn;

	c_character_t *fc = (c_character_t*)ct_get_nth(ecm_get(ref("character")), 0);
	c_side(fc)->side = side;


	if(side)
	{
		c_force_t *force = c_force(&fc->force_down);
		force->force = vec3_inv(force->force);
	}


	/* c_camera_t *cc = c_camera(ecm_get_camera(root.ecm)); */

	if(fc)
	{
		c_spacial_set_pos(c_spacial(fc), vec3(x, y - 0.5, z));
		c_spacial_rotate_Y(c_spacial(fc), -M_PI / 2);
		/* c_spacial_set_pos(c_spacial(cc->super.entity), 0, 0.7, 0); */
	}
	c_spacial_set_pos(c_spacial(&spawn), vec3(x, y - 0.5, z));

	return spawn;
}

static entity_t cmd_bridge(entity_t root, int argc, const char **argv)
{
#define order(N1, N2, n1, n2) ((n1 < n2) ? \
		(N1 = n1, N2 = n2) : \
		(N1 = n2, N2 = n1))

	float x1, y1, z1, x2, y2, z2, cx, cy, cz;
	int key;

	sscanf(argv[1], "%f", &x1);
	sscanf(argv[2], "%f", &y1);
	sscanf(argv[3], "%f", &z1);
	sscanf(argv[4], "%f", &x2);
	sscanf(argv[5], "%f", &y2);
	sscanf(argv[6], "%f", &z2);
	sscanf(argv[7], "%f", &cx);
	sscanf(argv[8], "%f", &cy);
	sscanf(argv[9], "%f", &cz);
	sscanf(argv[10], "%d", &key);

	entity_t bridge = entity_new(c_side_new(2, 1),
			c_bridge_new(),
			c_node_new());
	c_bridge_t *p = c_bridge(&bridge);

	order(p->ix1, p->ix2, x1, x2);
	order(p->iy1, p->iy2, y1, y2);
	order(p->iz1, p->iz2, z1, z2);

	p->ix1 -= cx; p->ix2 -= cx;
	p->iy1 -= cy; p->iy2 -= cy;
	p->iz1 -= cz; p->iz2 -= cz;

	p->min.x = p->ix1; p->min.y = p->iy1; p->min.z = p->iz1;
	p->max.x = p->ix2; p->max.y = p->iy2; p->max.z = p->iz2;

	p->min = vec3_sub(p->min, vec3(0.5));
	p->max = vec3_add(p->max, vec3(0.5));

	p->cx=cx;
	p->cy=cy;
	p->cz=cz;

	p->key=key;

	/* mesh_t *mesh = mesh_cuboid(0.5, */
			/* p->x1 - 0.01, p->y1 - 0.01, p->z1 - 0.01, */
			/* p->x2 + 0.01, p->y2 + 0.01, p->z2 + 0.01); */
	c_bridge_ready(p);

	c_spacial_set_pos(c_spacial(&bridge), vec3(p->cx, p->cy, p->cz));

	/* c_model(&bridge)->before_draw = (before_draw_cb)cmd_model_before_draw; */

	return bridge;
}

static mesh_t *g_portal;
static entity_t cmd_door(entity_t root, int argc, const char **argv)
{
	float x, y, z;
	char next[256];
	int dir;
	int side;

	sscanf(argv[1], "%f", &x);
	sscanf(argv[2], "%f", &y);
	sscanf(argv[3], "%f", &z);
	sscanf(argv[4], "%d", &dir);
	strcpy(next, argv[5]); 

	entity_t scene = c_state(&SYS)->scene;

	if(!g_portal)
	{
		g_portal = mesh_new();
		mesh_add_regular_quad(g_portal,
				vec3(0.0f, 0.0f, 0.3f), Z3, Z2, vec3(0.0f, 0.0f, -0.3f), Z3, Z2,
				vec3(0.0f, 0.9f, -0.3f), Z3, Z2, vec3(0.0f, 0.9f, 0.3f), Z3, Z2);
		g_portal->cull = 0;
	}

	entity_t grid = c_node_get_by_name(c_node(&scene), ref("grid"));
	side = c_grid_get(c_grid(&grid), x, y, z) & 1;

	mesh_t *door_mesh = sauces("door.obj");
	door_mesh->smooth_angle = 0;
	entity_t door = entity_new(c_name_new("door"),
			c_model_new(door_mesh, sauces("door.mat"), 1, 1),
			c_side_new(side, 0),
			c_node_new(),
			c_door_new(next));
	entity_t portal = entity_new(c_model_new(g_portal, mat_new("portal"), 0, 1));
	drawable_set_group(&c_model(&portal)->draw, ref("portal"));
	c_node_add(c_node(&door), 1, portal);
	/* c_model(&door)->before_draw = (before_draw_cb)cmd_model_before_draw; */

	c_spacial_set_pos(c_spacial(&door), vec3(x, y - 0.5 * (side ? -1:1), z));
	c_spacial_set_rot(c_spacial(&door), 0, 1, 0, dir * M_PI / 2);
	/* c_spacial_set_rot(c_spacial(door), 0, 1, 0, side ? -M_PI / 2.0 : 0); */

	return door;
}

static entity_t cmd_light(entity_t root, int argc, const char **argv)
{
	float x, y, z, intensity;
	int side;
	vec4_t color = vec4(0,0,0,1);

	sscanf(argv[1], "%f", &x);
	sscanf(argv[2], "%f", &y);
	sscanf(argv[3], "%f", &z);
	sscanf(argv[4], "%f", &intensity);
	sscanf(argv[5], "%f", &color.r);
	sscanf(argv[6], "%f", &color.g);
	sscanf(argv[7], "%f", &color.b);

	entity_t scene = c_state(&SYS)->scene;

	entity_t grid = c_node_get_by_name(c_node(&scene), ref("grid"));
	side = c_grid_get(c_grid(&grid), x, y, z) & 1;

	entity_t light = entity_new(c_name_new("light"),
			c_name_new("light"),
			c_side_new(side, 0),
			c_node_new(),
			c_side_follow_new(),
			c_light_new(20.0f, color, 1024));

	/* c_light(light)->before_draw = (before_draw_cb)cmd_light_before_draw; */

	c_spacial_set_pos(c_spacial(&light), vec3(x, y, z));

	return light;
}

void reg_custom_cmds()
{
	candle_reg_cmd("light", (cmd_cb)cmd_light);
	candle_reg_cmd("door", (cmd_cb)cmd_door);
	candle_reg_cmd("bridge", (cmd_cb)cmd_bridge);
	candle_reg_cmd("spawn", (cmd_cb)cmd_spawn);
	candle_reg_cmd("key", (cmd_cb)cmd_key);
	candle_reg_cmd("shift_grid", (cmd_cb)cmd_shift_grid);
}
