#pragma once
#ifndef ENEMY_H
#define ENEMY_H

#include <cstdint>

struct Enemy {
	std::string name;
	uint64_t pointer;
	uint64_t size;
	unsigned char* data;
};

#endif