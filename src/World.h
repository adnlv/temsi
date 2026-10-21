#pragma once
#include <glm/glm.hpp>

// All simulation quantities use SI units: metres, seconds, kilograms, kelvin.
namespace World
{
	constexpr float kHeight = 100.0e-9f;            // m, box height (100 nm)
	constexpr float kAspect = 16.0f / 9.0f;         // fixed box aspect ratio
	constexpr float kWidth  = kHeight * kAspect;    // m, ~177.8 nm
	constexpr glm::vec2 kHalfExtent{ kWidth / 2.0f, kHeight / 2.0f };

	// Effective hard-sphere radius. Argon's van der Waals radius is ~0.19 nm;
	// it is enlarged ~2x so collisions are visible.
	constexpr float kParticleRadius = 0.4e-9f;      // m

	constexpr float kPicosecond = 1.0e-12f;         // s
}
