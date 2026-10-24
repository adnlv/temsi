#include "Simulation.h"
#include <random>
#include <cmath>
#include <algorithm>

ParticleBuffer::ParticleBuffer()
	: num_max_particles(2000), num_active_particles(num_max_particles / 2)
{
	mass.resize(num_max_particles);
	radii.resize(num_max_particles);
	positions.resize(num_max_particles);
	velocities.resize(num_max_particles);
	colors.resize(num_max_particles);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> mass_dis(6.0e-26f, 7.0e-26f); // Mass of an Argon atom (~6.6e-26 kg)
	std::uniform_real_distribution<float> pos_x_dis(-World::kHalfExtent.x + World::kParticleRadius, World::kHalfExtent.x - World::kParticleRadius);
	std::uniform_real_distribution<float> pos_y_dis(-World::kHalfExtent.y + World::kParticleRadius, World::kHalfExtent.y - World::kParticleRadius);
	std::uniform_real_distribution<float> vel_dis(-400.0f, 400.0f); // m/s
	std::uniform_real_distribution<float> color_dis(0.2f, 1.0f);

	for (size_t i = 0; i < static_cast<size_t>(num_max_particles); ++i)
	{
		radii[i] = World::kParticleRadius;
		mass[i] = mass_dis(gen);
		velocities[i] = glm::vec2(vel_dis(gen), vel_dis(gen));
		colors[i] = glm::vec3(color_dis(gen), color_dis(gen), color_dis(gen));

		// Rejection sampling for non-overlapping initial placement
		glm::vec2 candidate_pos(0.0f);
		for (int attempt = 0; attempt < 100; ++attempt)
		{
			candidate_pos = glm::vec2(pos_x_dis(gen), pos_y_dis(gen));
			bool collision = false;
			for (size_t j = 0; j < i; ++j)
			{
				const glm::vec2 diff = candidate_pos - positions[j];
				const float min_dist = radii[i] + radii[j];
				if (glm::dot(diff, diff) < min_dist * min_dist)
				{
					collision = true;
					break;
				}
			}
			if (!collision)
			{
				break;
			}
		}
		positions[i] = candidate_pos;
	}
}

Simulation::Simulation()
{
	m_particle_next.resize(m_particles.num_max_particles);
	UpdateMetrics();
}

void Simulation::SetActiveParticles(int count) noexcept
{
	m_particles.num_active_particles = std::clamp(count, 0, m_particles.num_max_particles);
	UpdateMetrics();
}

void Simulation::Update(float dt)
{
	if (dt <= 0.0f)
	{
		return;
	}

	const size_t active_count = static_cast<size_t>(m_particles.num_active_particles);
	if (active_count == 0)
	{
		return;
	}

	const glm::vec2 limit = World::kHalfExtent;
	constexpr float cell_size = 2.0f * World::kParticleRadius;
	const int cols = static_cast<int>(std::ceil(2.0f * limit.x / cell_size));
	const int rows = static_cast<int>(std::ceil(2.0f * limit.y / cell_size));

	// Reset grid heads (zero heap allocations if cols*rows <= capacity)
	m_cell_head.assign(static_cast<std::vector<int>::size_type>(cols) * rows, -1);

	// 2. Populate the linked cell list O(N)
	for (size_t i = 0; i < active_count; ++i)
	{
		const auto& p{ m_particles.positions[i] };
		int gx = std::clamp(static_cast<int>((p.x + limit.x) / cell_size), 0, cols - 1);
		int gy = std::clamp(static_cast<int>((p.y + limit.y) / cell_size), 0, rows - 1);
		int cell_idx = gy * cols + gx;
		m_particle_next[i] = m_cell_head[cell_idx];
		m_cell_head[cell_idx] = static_cast<int>(i);
	}

	// Helper lambda to check and resolve collision between particles i and j
	auto check_collision = [&](size_t i, size_t j)
	{
		auto& p1{ m_particles.positions[i] };
		auto& v1{ m_particles.velocities[i] };
		const auto& r1{ m_particles.radii[i] };
		const auto& m1{ m_particles.mass[i] };

		auto& p2{ m_particles.positions[j] };
		auto& v2{ m_particles.velocities[j] };
		const auto& r2{ m_particles.radii[j] };
		const auto& m2{ m_particles.mass[j] };

		const auto pos_diff = p1 - p2;
		const auto min_dist = r1 + r2;
		const auto min_dist_sq = min_dist * min_dist;
		const auto dist_sq = glm::dot(pos_diff, pos_diff);
		if (dist_sq > min_dist_sq)
			return;

		const auto total_mass = m1 + m2;
		const float dist = std::sqrt(dist_sq);
		const glm::vec2 normal = dist > 0.0f ? (pos_diff / dist) : glm::vec2(1.0f, 0.0f);

		const float overlap = min_dist - dist;
		if (overlap > 0.0f)
		{
			p1 += normal * (overlap * (m2 / total_mass));
			p2 -= normal * (overlap * (m1 / total_mass));
		}

		const auto vel_diff = v1 - v2;
		const float rel_normal_vel = glm::dot(vel_diff, normal);
		if (rel_normal_vel >= 0.0f)
			return;

		const glm::vec2 impulse = rel_normal_vel * normal;
		v1 -= (2.0f * m2 / total_mass) * impulse;
		v2 += (2.0f * m1 / total_mass) * impulse;
	};

	// 3. Traverse grid cells and check collisions O(N)
	for (int gy = 0; gy < rows; ++gy)
	{
		for (int gx = 0; gx < cols; ++gx)
		{
			int cell_idx = gy * cols + gx;
			int i = m_cell_head[cell_idx];
			while (i != -1)
			{
				// Check against subsequent particles in the same cell
				int j = m_particle_next[i];
				while (j != -1)
				{
					check_collision(static_cast<size_t>(i), static_cast<size_t>(j));
					j = m_particle_next[j];
				}

				// Check against particles in the 4 forward neighboring cells (half-shell)
				constexpr int neighbor_offsets[4][2] = {
					{ 1,  0}, // Right
					{-1,  1}, // Bottom-Left
					{ 0,  1}, // Bottom
					{ 1,  1}  // Bottom-Right
				};

				for (const auto& offset : neighbor_offsets)
				{
					int nx = gx + offset[0];
					int ny = gy + offset[1];
					if (nx >= 0 && nx < cols && ny >= 0 && ny < rows)
					{
						int neighbor_cell_idx = ny * cols + nx;
						int nj = m_cell_head[neighbor_cell_idx];
						while (nj != -1)
						{
							check_collision(static_cast<size_t>(i), static_cast<size_t>(nj));
							nj = m_particle_next[nj];
						}
					}
				}

				i = m_particle_next[i];
			}
		}
	}

	for (size_t i = 0; i < active_count; ++i)
	{
		auto& p1{ m_particles.positions[i] };
		auto& v1{ m_particles.velocities[i] };
		const auto& r1{ m_particles.radii[i] };

		p1 += v1 * dt;

		if (p1.x - r1 < -limit.x)
		{
			p1.x = -limit.x + r1;
			v1.x *= -1.0f;
		}
		else if (p1.x + r1 > limit.x)
		{
			p1.x = limit.x - r1;
			v1.x *= -1.0f;
		}

		if (p1.y - r1 < -limit.y)
		{
			p1.y = -limit.y + r1;
			v1.y *= -1.0f;
		}
		else if (p1.y + r1 > limit.y)
		{
			p1.y = limit.y - r1;
			v1.y *= -1.0f;
		}
	}

	UpdateMetrics();
}

void Simulation::UpdateMetrics()
{
	const size_t active_count = static_cast<size_t>(m_particles.num_active_particles);
	glm::vec2 current_total_momentum{ 0.0f };
	float current_total_kinetic_energy = 0.0f;

	for (size_t i = 0; i < active_count; ++i)
	{
		const auto& v1{ m_particles.velocities[i] };
		const auto& m1{ m_particles.mass[i] };
		const float velocity_sq = glm::dot(v1, v1);
		current_total_kinetic_energy += m1 * velocity_sq / 2.0f;
		current_total_momentum += v1 * m1;
	}

	m_total_kinetic_energy = current_total_kinetic_energy;
	m_avg_kinetic_energy = active_count != 0
		? m_total_kinetic_energy / static_cast<float>(active_count)
		: 0.0f;
	m_total_momentum = current_total_momentum;

	m_temperature.update_from_kinetic_energy(m_avg_kinetic_energy);

	ComputeSpeedDistribution();
}

void Simulation::ComputeSpeedDistribution()
{
	const size_t active_count = static_cast<size_t>(m_particles.num_active_particles);
	m_speed_dist.empirical_pdf.fill(0.0f);
	m_speed_dist.theoretical_pdf.fill(0.0f);
	m_speed_dist.max_pdf_value = 0.0f;
	m_speed_dist.peak_speed_theoretical = 0.0f;
	m_speed_dist.mean_speed_theoretical = 0.0f;
	m_speed_dist.rms_speed_theoretical = 0.0f;

	if (active_count == 0)
	{
		return;
	}

	constexpr float bin_width = SpeedDistribution::kBinWidth;
	constexpr int num_bins = SpeedDistribution::kNumBins;

	// 1. Accumulate empirical histogram counts
	std::array<int, num_bins> bin_counts{};
	float total_mass = 0.0f;

	for (size_t i = 0; i < active_count; ++i)
	{
		const float speed = glm::length(m_particles.velocities[i]);
		int bin = static_cast<int>(speed / bin_width);
		if (bin >= 0 && bin < num_bins)
		{
			bin_counts[bin]++;
		}
		total_mass += m_particles.mass[i];
	}

	const float avg_mass = total_mass / static_cast<float>(active_count);
	const float T = m_temperature.kelvin();
	const float kB = Temperature::BoltzmannConstant();

	// 2. Normalize empirical PDF and calculate theoretical 2D Rayleigh PDF
	// 2D Maxwell-Boltzmann: f(v) = (m*v / (kB*T)) * exp(-m*v^2 / (2*kB*T))
	const float inv_active_particles = 1.0f / static_cast<float>(active_count);
	const bool valid_temp = (T > 0.0f && avg_mass > 0.0f);

	if (valid_temp)
	{
		m_speed_dist.peak_speed_theoretical = std::sqrt(kB * T / avg_mass);
		m_speed_dist.mean_speed_theoretical = std::sqrt(3.1415926535f * kB * T / (2.0f * avg_mass));
		m_speed_dist.rms_speed_theoretical = std::sqrt(2.0f * kB * T / avg_mass);
	}

	float max_val = 0.0f;
	for (int b = 0; b < num_bins; ++b)
	{
		// Empirical probability density = fraction of particles in bin / bin_width
		const float empirical_density = (static_cast<float>(bin_counts[b]) * inv_active_particles) / bin_width;
		m_speed_dist.empirical_pdf[b] = empirical_density;
		max_val = std::max(max_val, empirical_density);

		if (valid_temp)
		{
			const float v = (static_cast<float>(b) + 0.5f) * bin_width;
			const float exponent = -(avg_mass * v * v) / (2.0f * kB * T);
			const float theoretical_density = ((avg_mass * v) / (kB * T)) * std::exp(exponent);
			m_speed_dist.theoretical_pdf[b] = theoretical_density;
			max_val = std::max(max_val, theoretical_density);
		}
	}

	m_speed_dist.max_pdf_value = max_val;
}
