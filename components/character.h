#ifndef CHARACTER_H
#define CHARACTER_H

#include <ecm.h>

typedef struct
{
	c_t super; /* extends c_t */

	int forward, backward, left, right, jump, swap;
	float targR;
	float max_jump_vel;

	entity_t orientation;

	vec3_t up;

	int plane_movement;

	entity_t force_down;
} c_character_t;

extern unsigned long ct_character;

DEF_CASTER(ct_character, c_character, c_character_t);

c_character_t *c_character_new(entity_t orientation, int plane_movement, entity_t force_down);
void c_character_register(ecm_t *ecm);

#endif /* !CHARACTER_H */