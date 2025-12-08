#include "util/file.hpp"

#include <fstream>

namespace util
{
	std::expected<std::vector<std::byte>, util::Error> read_file(
		const std::filesystem::path& path,
		size_t max_size
	) noexcept
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file.is_open()) return util::Error(std::format("Open file '{}' failed", path.string()));
		file.exceptions(std::ios::failbit | std::ios::badbit);

		try
		{
			const size_t file_size = static_cast<size_t>(file.tellg());
			if (file_size > max_size)
				return util::Error(
					std::format(
						"File '{}' has size of {}B exceeds maximum {}B",
						path.string(),
						file_size,
						max_size
					)
				);

			std::vector<std::byte> data(file_size);
			file.seekg(0, std::ios::beg);
			file.read(reinterpret_cast<char*>(data.data()), file_size);

			return std::move(data);
		}
		catch (const std::exception& e)
		{
			return util::Error(std::format("Read file '{}' failed: {}", path.string(), e.what()));
		}
	}

	std::expected<void, util::Error> write_file(
		const std::filesystem::path& path,
		std::span<const std::byte> data
	) noexcept
	{
		std::ofstream file(path, std::ios::binary | std::ios::trunc);
		if (!file.is_open()) return util::Error(std::format("Open file '{}' failed", path.string()));
		file.exceptions(std::ios::failbit | std::ios::badbit);

		try
		{
			file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
		}
		catch (const std::exception& e)
		{
			return util::Error(std::format("Write file '{}' failed: {}", path.string(), e.what()));
		}

		return {};
	}
}