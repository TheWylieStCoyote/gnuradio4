#ifndef GNURADIO_SIGMF_SOURCE_HPP
#define GNURADIO_SIGMF_SOURCE_HPP

#include <complex>
#include <fstream>
#include <sstream>
#include <string>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::fileio {

GR_REGISTER_BLOCK(gr::blocks::fileio::SigMFSource, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct SigMFSource : gr::Block<SigMFSource<T>> {
    using Description = Doc<R""(
@brief SigMF source — reads samples from a `.sigmf-data` binary file.

Opens `file_name.sigmf-data` and optionally reads `sample_rate` from the companion
`.sigmf-meta` JSON file. Streams raw little-endian samples as type T. When `repeat`
is true the file rewinds after the last sample; otherwise `requestStop()` is called
at EOF.
)"">;

    PortOut<T> out;

    Annotated<std::string, "file_name",   Doc<"base path (without .sigmf-data/.sigmf-meta extension)">, Visible> file_name{"input"};
    Annotated<bool,        "repeat",      Doc<"rewind and replay from start at EOF">>                             repeat{false};

    GR_MAKE_REFLECTABLE(SigMFSource, out, file_name, repeat);

    std::ifstream  _dataFile{};
    std::streampos _dataStart{0};
    double         _parsedSampleRate{1e6};

    double parsedSampleRate() const noexcept { return _parsedSampleRate; }

    std::expected<void, gr::Error> start() {
        const std::string base = static_cast<const std::string&>(file_name);
        _dataFile.open(base + ".sigmf-data", std::ios::binary);
        if (!_dataFile.is_open()) {
            return std::unexpected(gr::Error{std::format("SigMFSource: cannot open '{}.sigmf-data'", base)});
        }
        _dataStart        = _dataFile.tellg();
        _parsedSampleRate = 1e6; // default

        // attempt to read sample_rate from metadata
        std::ifstream meta(base + ".sigmf-meta");
        if (meta.is_open()) {
            std::string content{std::istreambuf_iterator<char>(meta), {}};
            const std::string key = "\"core:sample_rate\":";
            const auto pos = content.find(key);
            if (pos != std::string::npos) {
                const auto numStart = pos + key.size();
                // skip whitespace
                auto i = numStart;
                while (i < content.size() && (content[i] == ' ' || content[i] == '\t')) ++i;
                std::istringstream ss(content.substr(i));
                double sr{};
                if (ss >> sr && sr > 0) _parsedSampleRate = sr;
            }
        }
        return {};
    }

    std::expected<void, gr::Error> stop() {
        if (_dataFile.is_open()) _dataFile.close();
        return {};
    }

    [[nodiscard]] T processOne() noexcept {
        if (!_dataFile.is_open()) return T{};
        T sample{};
        if (!_dataFile.read(reinterpret_cast<char*>(&sample), sizeof(T))) {
            if (static_cast<bool>(repeat)) {
                _dataFile.clear();
                _dataFile.seekg(_dataStart);
                if (!_dataFile.read(reinterpret_cast<char*>(&sample), sizeof(T))) return T{};
            } else {
                this->requestStop();
                return T{};
            }
        }
        return sample;
    }
};

} // namespace gr::blocks::fileio

#endif // GNURADIO_SIGMF_SOURCE_HPP
