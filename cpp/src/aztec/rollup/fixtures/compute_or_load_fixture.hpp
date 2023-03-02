#pragma once
#include <fstream>
#include <functional>
#include <sys/stat.h>
#include <common/serialize.hpp>
#include <common/log.hpp>
#include <common/throw_or_abort.hpp>
#include <filesystem>

namespace rollup {
namespace fixtures {

inline bool exists(std::string const& path)
{
    struct stat st;
    return (stat(path.c_str(), &st) != -1);
}

inline std::vector<uint8_t> compute_or_load_fixture(std::string const& path,
                                                    std::string const& name,
                                                    std::function<std::vector<uint8_t>()> const& f)
{
    // Tests are being run from build directory.
    auto filename = path + "/" + name;
    if (exists(filename)) {
        auto stream = std::ifstream(filename);
        std::vector<uint8_t> data;
        read(stream, data);
        info("Loaded fixture: ", filename);
        return data;
    } else {
        info("Computing fixture: ", name, "...");
        auto data = f();
        if (data.size()) {
            std::filesystem::create_directories(path.c_str());
            auto stream = std::ofstream(filename);
            write(stream, data);
            if (!stream.good()) {
                throw_or_abort(format("Failed to write: ", filename));
            }
        }
        return data;
    }
}

} // namespace fixtures
} // namespace rollup
