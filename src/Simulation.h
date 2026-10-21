#pragma once
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include "World.h"

class Temperature
{
public:
	[[nodiscard]] float kelvin() const noexcept { return m_kelvin; }
	[[nodiscard]] float celsius() const noexcept { return m_kelvin + k_C_ABS_ZERO; }
	[[nodiscard]] float fahrenheit() const noexcept { return m_kelvin * k_K_TO_F_RATIO + k_F_ABS_ZERO; }

	void update_from_kinetic_energy(float energy) noexcept { m_kelvin = energy / k_B; }

	static constexpr float BoltzmannConstant() noexcept { return k_B; }

private:
	// Boltzmann constant
	static constexpr float k_B{ 1.380649e-23f };

	static constexpr float k_C_ABS_ZERO{ -273.15f };
	static constexpr float k_F_ABS_ZERO{ -459.67f };
	static constexpr float k_K_TO_F_RATIO{ 1.8f };

	float m_kelvin{ 0.0f };
};

struct ParticleBuffer
{
	std::vector<float> mass;
	std::vector<float> radii;
	std::vector<glm::vec2> positions;
	std::vector<glm::vec2> velocities;
	std::vector<glm::vec3> colors;

	int num_max_particles;
	int num_active_particles;

	ParticleBuffer();
	~ParticleBuffer() = default;
};

struct SpeedDistribution
{
	static constexpr int kNumBins = 30;
	static constexpr float kMaxSpeed = 1200.0f; // m/s
	static constexpr float kBinWidth = kMaxSpeed / static_cast<float>(kNumBins); // 40 m/s

	std::array<float, kNumBins> empirical_pdf{};
	std::array<float, kNumBins> theoretical_pdf{};
	float max_pdf_value{ 0.0f };
	float peak_speed_theoretical{ 0.0f };
	float mean_speed_theoretical{ 0.0f };
	float rms_speed_theoretical{ 0.0f };
};

class Simulation
{
public:
	Simulation();
	~Simulation() = default;

	void Update(float dt);

	[[nodiscard]] const ParticleBuffer& Particles() const noexcept { return m_particles; }
	[[nodiscard]] ParticleBuffer& Particles() noexcept { return m_particles; }

	[[nodiscard]] const Temperature& GetTemperature() const noexcept { return m_temperature; }
	[[nodiscard]] float AvgKineticEnergy() const noexcept { return m_avg_kinetic_energy; }
	[[nodiscard]] float TotalKineticEnergy() const noexcept { return m_total_kinetic_energy; }
	[[nodiscard]] glm::vec2 TotalMomentum() const noexcept { return m_total_momentum; }

	[[nodiscard]] int MaxParticles() const noexcept { return m_particles.num_max_particles; }
	[[nodiscard]] int ActiveParticles() const noexcept { return m_particles.num_active_particles; }
	void SetActiveParticles(int count) noexcept;

	[[nodiscard]] const SpeedDistribution& GetSpeedDistribution() const noexcept { return m_speed_dist; }

private:
	ParticleBuffer m_particles;
	Temperature m_temperature;
	SpeedDistribution m_speed_dist;

	float m_avg_kinetic_energy{ 0.0f };
	float m_total_kinetic_energy{ 0.0f };
	glm::vec2 m_total_momentum{ 0.0f };

	std::vector<glm::vec2> m_pending_impulses;
	std::vector<int> m_cell_head;
	std::vector<int> m_particle_next;

	void ComputeSpeedDistribution();
};
