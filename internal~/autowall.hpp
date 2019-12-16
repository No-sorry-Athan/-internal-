#pragma once
#include "helpers/math.hpp"
#include "helpers/utils.hpp"
#include "valve_sdk/csgostructs.hpp"

struct FireBulletData {
	FireBulletData(const Vector& eyePos) : src(eyePos) {}
	Vector           src;
	trace_t          enter_trace;
	Vector           direction;
	CTraceFilter    filter;
	float            trace_length;
	float            trace_length_remaining;
	float            current_damage;
	int              penetrate_count;
};

bool CanHit(const Vector& point, float* damage_given);
Vector bestPoint(C_BasePlayer* entity);

