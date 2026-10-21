#pragma once
#include <filesystem>
#include <vector>
#include <stdexcept>

#if defined(_WIN32)
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#elif !defined(__linux__)
	#error "Paths.h: only Windows and Linux are supported"
#endif

namespace Paths
{
	[[nodiscard]] inline std::filesystem::path ExecutableDir()
	{
#if defined(_WIN32)
		std::vector<wchar_t> buf(MAX_PATH);
		for (;;)
		{
			DWORD n = GetModuleFileNameW(nullptr, buf.data(), static_cast<DWORD>(buf.size()));
			if (n == 0) throw std::runtime_error("Paths: GetModuleFileNameW failed");
			if (n < buf.size()) return std::filesystem::path(buf.data(), buf.data() + n).parent_path();
			buf.resize(buf.size() * 2);
		}
#else // Linux
		return std::filesystem::read_symlink("/proc/self/exe").parent_path();
#endif
	}

	[[nodiscard]] inline const std::filesystem::path& AssetsDir()
	{
		static const std::filesystem::path dir = ExecutableDir() / "assets";
		return dir;
	}
}
