#include <ecm.h>
#include <candle.h>
#include <components/rigid_body.h>
#include <components/name.h>
#include <components/model.h>
#include <components/spacial.h>
#include <components/node.h>
#include "grid.h"
#include "side.h"
#include <stdlib.h>

void mesh_add_spike(mesh_t *self, float s, vec3_t v, vec3_t dir, int inverted_normals);
/* void mesh_add_spike(mesh_t *self, float s, float x, float y, float z, */
		/* int px, int py, int pz, int inverted_normals); */
void mesh_add_plane(mesh_t *self, float s, vec3_t v, vec3_t dir, int inverted_normals);
/* void mesh_add_plane(mesh_t *self, float s, float x, float y, float z, */
		/* int px, int py, int pz, int inverted_normals); */


DEC_CT(ct_grid);
DEC_SIG(grid_update);

typedef void(*create_cb)(mesh_t *self, float s, vec3_t v, vec3_t dir,
		int inverted_normals);
/* typedef void(*create_cb)(mesh_t *self, float s, float x, float y, float z, */
		/* int px, int py, int pz, int inverted_normals); */

static int c_grid_update(c_grid_t *self);
static mesh_t *mesh_from_grid(c_grid_t *grid, int flags, ...);
int plane_to_side(mesh_t *mesh, int val0, int flag, c_grid_t *grid,
		vec3_t v, vec3_t dir, create_cb create);

int template_model_before_draw(c_model_t *self);

void c_grid_init(c_grid_t *self)
{
	self->super = component_new(ct_grid);
	self->map = NULL;
}

float c_rigid_body_grid_collider(c_rigid_body_t *self, vec3_t pos)
{
	
	int side = c_side(c_ecm(self)->common)->side;

	c_grid_t *g = c_grid(c_entity(self));
	int val = c_grid_get(g, round(pos.x), round(pos.y), round(pos.z));

	return ((val&1) != side || val == -1) ? 0 : -1;
}

c_grid_t *c_grid_new(ecm_t *ecm, int mx, int my, int mz)
{
	c_grid_t *self = malloc(sizeof *self);
	c_grid_init(self);

	self->mx = mx;
	self->my = my;
	self->mz = mz;
	self->map = calloc(mx * my * mz, sizeof(*self->map));

	self->blocks = entity_new(ecm, 3, c_name_new("blocks"), c_side_new(0),
			/* c_model_new(NULL, candle_material_get(candle, "pack1/white"), 1)); */
			c_model_paint(c_model_new(NULL, 1), 0, candle_material_get(candle,
						"pack1/white")));

	self->cage = entity_new(ecm, 3, c_name_new("cage"), c_side_new(0),
			c_model_paint(c_model_new(NULL, 1), 0,
				candle_material_get(candle, "pack1/piramids")));

	material_t *stone3 = candle_material_get(candle, "pack1/stone3");
	stone3->diffuse.color = vec4(0.6f, 0.1f, 0.14f, 1.0f);
	stone3->diffuse.texture_blend = 0.5;
	stone3->normal.texture_blend = 0.3;

	self->boxes = entity_new(ecm, 3, c_name_new("movable"), c_side_new(0),
			c_model_paint(c_model_new(NULL, 1), 0, stone3));

	self->blocks_inv = entity_new(ecm, 3, c_name_new("bloc_i"), c_side_new(1),
			c_model_paint(c_model_new(NULL, 1), 0,
				candle_material_get(candle, "pack1/piramids")));

	self->cage_inv = entity_new(ecm, 3, c_name_new("cage_i"), c_side_new(1),
			c_model_paint(c_model_new(NULL, 1), 0,
				candle_material_get(candle, "pack1/white")));

	self->boxes_inv = entity_new(ecm, 3, c_name_new("movab_i"), c_side_new(1),
			c_model_paint(c_model_new(NULL, 1), 0,
				candle_material_get(candle, "movable")));

	c_model(self->blocks_inv)->before_draw =
		c_model(self->cage_inv)->before_draw =
		c_model(self->boxes_inv)->before_draw =
		(before_draw_cb)template_model_before_draw;

	c_model(self->blocks)->before_draw =
		c_model(self->cage)->before_draw =
		c_model(self->boxes)->before_draw =
		(before_draw_cb)template_model_before_draw;


	return self;
}

void mesh_add_spike(mesh_t *mesh, float s, vec3_t v, vec3_t dir, int inverted_normals)
{
	/* return; */
	mesh_save(mesh);

	mesh_translate(mesh, v.x, v.y, v.z);
    if(dir.x > 0)
	{
		mesh_rotate(mesh, 90, 0, 0, 1);
	}
	else if(dir.x < 0)
	{
		mesh_rotate(mesh, -90, 0, 0, 1);
	}
	else if(dir.y > 0)
	{
		mesh_rotate(mesh, 180, 1, 0, 0);
	}
	else if(dir.y < 0)
	{
		/* mesh_rotate(mesh, 180, 0, 0, 0); */
	}
	else if(dir.z > 0)
	{
		mesh_rotate(mesh, -90, 1, 0, 0);
	}
	else if(dir.z < 0)
	{
		mesh_rotate(mesh, 90, 1, 0, 0);
	}
	mesh_translate(mesh, 0, -0.5, 0);

	int v1, v2, v3;
	/* mesh_add_plane(mesh, s, 0, 0, 0, 0, -1, 1); */
	/* mesh_restore(mesh); */
	/* return; */

	float i, j;
	int U, V;

	/* float wid = 0.25; */
	const float wid = 0.5;
	const float hei = 0.6;
    for(i=-wid, U=-2; i<wid; i+=wid,U++)
		for(j=-wid, V=-2; j<wid; j+=wid,V++)
	{

		float mainx = i + wid/2;
		float mainy = (U + V) & 1 ? hei : -hei;
		float mainz = j + wid/2;

		vec3_t zero = vec3(0.0);

		vec3_t P1 = vec3(mainx, mainy, mainz);
		vec3_t P2 = vec3(i, 0, j+wid);
		vec3_t P3 = vec3(i+wid, 0, j+wid);
		v1 = mesh_add_vert(mesh, P1);
		v2 = mesh_add_vert(mesh, P2);
		v3 = mesh_add_vert(mesh, P3);
		mesh_add_triangle(mesh, 
				v1, zero, vec2(P1.x, P1.z),
				v2, zero, vec2(P2.x, P2.z),
				v3, zero, vec2(P3.x, P3.z), 1);

		P1 = vec3(mainx, mainy, mainz);
		P2 = vec3(i+wid, 0, j+wid);
		P3 = vec3(i+wid, 0, j);
		v1 = mesh_add_vert(mesh, P1);
		v2 = mesh_add_vert(mesh, P2);
		v3 = mesh_add_vert(mesh, P3);
		mesh_add_triangle(mesh,
				v1, zero, vec2(P1.x, P1.z),
				v2, zero, vec2(P2.x, P2.z),
				v3, zero, vec2(P3.x, P3.z), 1);

		P1 = vec3(mainx, mainy, mainz);
		P2 = vec3(i+wid, 0, j);
		P3 = vec3(i, 0, j);
		v1 = mesh_add_vert(mesh, P1);
		v2 = mesh_add_vert(mesh, P2);
		v3 = mesh_add_vert(mesh, P3);
		mesh_add_triangle(mesh,
				v1, zero, vec2(P1.x, P1.z),
				v2, zero, vec2(P2.x, P2.z),
				v3, zero, vec2(P3.x, P3.z), 1);

		P1 = vec3(mainx, mainy, mainz);
		P2 = vec3(i, 0, j);
		P3 = vec3(i, 0, j+wid);
		v1 = mesh_add_vert(mesh, P1);
		v2 = mesh_add_vert(mesh, P2);
		v3 = mesh_add_vert(mesh, P3);
		mesh_add_triangle(mesh,
				v1, zero, vec2(P1.x, P1.z),
				v2, zero, vec2(P2.x, P2.z),
				v3, zero, vec2(P3.x, P3.z), 1);

	}
	mesh_restore(mesh);
	mesh_modified(mesh);
}


void mesh_add_plane(mesh_t *self, float s, vec3_t v, vec3_t dir, int invert_normals)
{
	int v1, v2, v3, v4;

	mesh_save(self);

	mesh_translate(self, v.x, v.y, v.z);
    if(dir.x > 0)
	{
		mesh_rotate(self, 90, 0, 0, 1);
		mesh_rotate(self, 90, 0, 1, 0);
	}
	else if(dir.x < 0)
	{
		mesh_rotate(self, -90, 0, 0, 1);
		mesh_rotate(self, -90, 0, 1, 0);
	}
	else if(dir.y > 0)
	{
		mesh_rotate(self, 180, 1, 0, 0);
	}
	else if(dir.y < 0)
	{
		/* mesh_rotate(self, 180, 1, 0, 0); */
	}
	else if(dir.z > 0)
	{
		mesh_rotate(self, -90, 1, 0, 0);
	}
	else if(dir.z < 0)
	{
		mesh_rotate(self, 90, 1, 0, 0);
		mesh_rotate(self, 180, 0, 1, 0);
	}

	vec2_t v1t, v2t, v3t, v4t;
		v1t = vec2(+s, -s);
		v2t = vec2(-s, -s);
		v3t = vec2(-s, +s);
		v4t = vec2(+s, +s);
		v1 = mesh_add_vert(self, vec3(+s, -s, -s));
		v2 = mesh_add_vert(self, vec3(-s, -s, -s));
		v3 = mesh_add_vert(self, vec3(-s, -s, +s));
		v4 = mesh_add_vert(self, vec3(+s, -s, +s));
	mesh_add_quad(self,
			v1, vec3(0.0), v1t,
			v2, vec3(0.0), v2t,
			v3, vec3(0.0), v3t,
			v4, vec3(0.0), v4t);
	mesh_restore(self);
	mesh_modified(self);
}

static int c_grid_created(c_grid_t *self)
{
	c_grid_update(self);

	entity_t e = c_entity(self);
	c_node_add(c_node(e), 6, self->boxes_inv, self->cage_inv,
			self->blocks_inv, self->boxes, self->cage, self->blocks);

	entity_add_component(e,
			(c_t*)c_rigid_body_new((collider_cb)c_rigid_body_grid_collider));

	return 1;
}

void c_grid_set(c_grid_t *self, int x, int y, int z, int val)
{
	if(x < 0 || x >= self->mx ||
			y < 0 || y >= self->my ||
			z < 0 || z >= self->mz) return;

	self->map[x + (y * self->mz + z) * self->mx] = val;
}

int c_grid_get(c_grid_t *self, int x, int y, int z)
{
	if(x < 0 || x >= self->mx ||
			y < 0 || y >= self->my ||
			z < 0 || z >= self->mz) return -1;

	return self->map[z + (y * self->mz + x) * self->mx];
}

void c_grid_register(ecm_t *ecm)
{
	ct_t *ct = ecm_register(ecm, "Grid", &ct_grid, sizeof(c_grid_t),
			(init_cb)c_grid_init, 0);

	ecm_register_signal(ecm, &grid_update, 0);

	ct_register_listener(ct, WORLD, grid_update, (signal_cb)c_grid_update);
	/* ct_register_listener(ct, WORLD, collider_callback, */
			/* (signal_cb)c_grid_collider); */
	ct_register_listener(ct, SAME_ENTITY, entity_created,
			(signal_cb)c_grid_created);
}

static int c_grid_update(c_grid_t *self)
{
	mesh_t *new_terrainA = mesh_from_grid(self, 2, 0x4 | 0x0, mesh_add_spike,
			0x0 | 0x0, mesh_add_plane);

	mesh_t *new_terrainB = mesh_from_grid(self, 2, 0x4 | 0x1, mesh_add_spike,
			0x0 | 0x1, mesh_add_plane);

	mesh_t *new_cageA = mesh_from_grid(self, 1, -2 | 0x0, mesh_add_plane);

	mesh_t *new_cageB = mesh_from_grid(self, 1, -2 | 0x1, mesh_add_plane);

	mesh_t *new_boxesA = mesh_from_grid(self, 1, 0x2 | 0x0, mesh_add_plane);

	mesh_t *new_boxesB = mesh_from_grid(self, 1, 0x2 | 0x1, mesh_add_plane);

	/* mesh_cull_face(new_terrainB, 1); */
	/* mesh_cull_face(new_boxesB, 1); */

	c_model_set_mesh(c_model(self->blocks), new_terrainA);
	c_model_set_mesh(c_model(self->cage), new_cageA);
	c_model_set_mesh(c_model(self->boxes), new_boxesA);

	c_model_set_mesh(c_model(self->blocks_inv), new_terrainB);
	c_model_set_mesh(c_model(self->cage_inv), new_cageB);
	c_model_set_mesh(c_model(self->boxes_inv), new_boxesB);

	entity_signal(self->super.entity, spacial_changed, &self->blocks);

	/* c_grid_print(self); */

	return 1;
}

void c_grid_print(c_grid_t *self)
{
	int x, y, z;
    for(x = 0; x < self->mx; x++)
	{
		for(y = 0; y < self->my; y++)
		{
			for(z = 0; z < self->mz; z++)
			{
				int val = c_grid_get(self, x, y, z);
				printf("%d ", val);
			}
			printf("\n");
		}
		printf("\n");
	}
}

mesh_t *mesh_from_grid(c_grid_t *grid, int flags, ...)
{
	mesh_t *self = mesh_new();
	/* self->wireframe = 1; */
	int x, y, z;
	mesh_lock(self);

	va_list list;

    for(x = 0; x < grid->mx; x++)
		for(y = 0; y < grid->my; y++)
			for(z = 0; z < grid->mz; z++)
	{
		int val = c_grid_get(grid, x, y, z);

		int j, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0;
		va_start(list, flags);
		for(j = 0; j < flags; j++)
		{
			int flag = va_arg(list, int);
			int side = flag & (0x1);
			flag = flag & (~0x1);

			create_cb create = va_arg(list, create_cb);

			if((val & 1) != side) continue;

			if((val & ~0x1) == flag || flag == 2 || flag == -2 || (val & ~0x1) == 0x4)
			{
				if(!r1) r1 = plane_to_side(self, val, flag, grid, vec3(x, y, z), vec3(-1,  0,  0), create);
				if(!r2) r2 = plane_to_side(self, val, flag, grid, vec3(x, y, z), vec3( 0, -1,  0), create);
				if(!r3) r3 = plane_to_side(self, val, flag, grid, vec3(x, y, z), vec3( 0,  0, -1), create);

				if(!r4) r4 = plane_to_side(self, val, flag, grid, vec3(x, y, z), vec3( 1,  0,  0), create);
				if(!r5) r5 = plane_to_side(self, val, flag, grid, vec3(x, y, z), vec3( 0,  1,  0), create);
				if(!r6) r6 = plane_to_side(self, val, flag, grid, vec3(x, y, z), vec3( 0,  0,  1), create);
			}
		}
		va_end(list);
	}
	mesh_translate_uv(self, vec2(0.5, 0.5));
	/* mesh_scale_uv(self, 2); */
	/* mesh_scale_uv(self, 0.5); */

	mesh_unlock(self);
	return self;
}

int plane_to_side(mesh_t *mesh, int val0, int flag, c_grid_t *grid,
		vec3_t v, vec3_t dir, create_cb create)
{
	int val = c_grid_get(grid, v.x + dir.x, v.y + dir.y, v.z + dir.z);
	if((val & 0x1) == (val0 & 0x1) && flag != -2) return 0;

	if(flag == 2)
	{
		if((val0 & 2) != (val & 2) && val != -1)
		{
			create(mesh, 0.5, v, dir, 0);
			return 1;
		}
		return 0;
	}
	if(val == -1 && flag == -2)
	{
		create(mesh, 0.5, v, dir, 0);
		return 1;
	}
	if((val & ~0x1) == flag)
	{
		create(mesh, 0.5, v, dir, 0);
		return 1;
	}
	if(flag == 0 && val != -1 && !(val&2))
	{
		create(mesh, 0.5, v, dir, 0);
		return 1;
	}
	return 0;
}

int grid_transverse(c_grid_t *self, vec3_t curpos, vec3_t raydir)
{

	int retval = -1;
	// setup 3DDDA (double check reusability of primary ray data)
	vec3_t cb, tmax, tdelta = vec3(0.0), cell;
	cell = curpos;
	int stepX, outX, X = (int)cell.x;
	int stepY, outY, Y = (int)cell.y;
	int stepZ, outZ, Z = (int)cell.z;
	if ((X < 0) || (X >= self->mx) || (Y < 0) || (Y >= self->my) || (Z < 0) || (Z >= self->mz)) return 0;
	if (raydir.x > 0)
	{
		stepX = 1, outX = self->mx;
		cb.x = (X + 1) * self->mx;
	}
	else 
	{
		stepX = -1, outX = -1;
		cb.x = X;
	}
	if (raydir.y > 0.0f)
	{
		stepY = 1, outY = self->my;
		cb.y = (Y + 1); 
	}
	else 
	{
		stepY = -1, outY = -1;
		cb.y = Y;
	}
	if (raydir.z > 0.0f)
	{
		stepZ = 1, outZ = self->mz;
		cb.z = (Z + 1);
	}
	else 
	{
		stepZ = -1, outZ = -1;
		cb.z = Z;
	}
	float rxr, ryr, rzr;
	if (raydir.x != 0)
	{
		rxr = 1.0f / raydir.x;
		tmax.x = (cb.x - curpos.x) * rxr; 
		tdelta.x = stepX * rxr;
	}
	else tmax.x = 1000000;

	if (raydir.y != 0)
	{
		ryr = 1.0f / raydir.y;
		tmax.y = (cb.y - curpos.y) * ryr; 
		tdelta.y = stepY * ryr;
	}
	else tmax.y = 1000000;

	if (raydir.z != 0)
	{
		rzr = 1.0f / raydir.z;
		tmax.z = (cb.z - curpos.z) * rzr; 
		tdelta.z = stepZ * rzr;
	}
	else tmax.z = 1000000;

	// start stepping
	int val = 0;
	// trace primary ray
	while (1)
	{
		val = c_grid_get(self, X, Y, Z);
		if(val)
		{
			return val;
		}
		if (tmax.x < tmax.y)
		{
			if (tmax.x < tmax.z)
			{
				X = X + stepX;
				if (X == outX) return -1;
				tmax.x += tdelta.x;
			}
			else
			{
				Z = Z + stepZ;
				if (Z == outZ) return -1;
				tmax.z += tdelta.z;
			}
		}
		else
		{
			if (tmax.y < tmax.z)
			{
				Y = Y + stepY;
				if (Y == outY) return -1;
				tmax.y += tdelta.y;
			}
			else
			{
				Z = Z + stepZ;
				if (Z == outZ) return -1;
				tmax.z += tdelta.z;
			}
		}
	}
	return retval;
}
