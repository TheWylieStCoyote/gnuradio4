#ifndef GNURADIO_SIGMF_SINK_HPP
#define GNURADIO_SIGMF_SINK_HPP

#include <complex>
#include <cstdint>
#include <fstream>
#include <string>
#include <type_traits>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::fileio {

GR_REGISTER_BLOCK(gr::blocks::fileio::SigMFSink, [T], [ float, double, std::complex<float>, std::complex<double> ])

namespace detail {

template<typename T>
[[nodiscard]] constexpr std::string_view sigmfDatatype() noexcept {
    if constexpr (std::is_same_v<T, std::complex<float>>)  return "cf32_le";
    if constexpr (std::is_same_v<T, std::complex<double>>) return "cf64_le";
    if constexpr (std::is_same_v<T, float>)                return "rf32_le";
    if constexpr (std::is_same_v<T, double>)               return "rf64_le";
    return "rf32_le";
}

} // namespace detail

template<typename T>
struct SigMFSink : gr::Block<SigMFSink<T>> {
    using Description = Doc<R""(
@brief SigMF sink — writes a sample stream to a `.sigmf-data` binary file
with a companion `.sigmf-meta` JSON metadata file.

The binary file contains raw little-endian samples in the format implied by
T: `cf32_le` for complex<float>, `cf64_le` for complex<double>, `rf32_le`
for float, `rf64_le` for double. The metadata file is written on `stop()`.
)"">;

    PortIn<T> in;

    Annotated<std::string, "file_name",   Doc<"base file name (without .sigmf-data/.sigmf-meta extension)">, Visible> file_name{"output"};
    Annotated<double,      "sample_rate", Doc<"sample rate to embed in metadata">,                           Visible> sample_rate{1e6};
    Annotated<std::string, "author",  Doc<"optional author field for the metadata">>      author{""};
    Annotated<std::string, "detail",  Doc<"optional description string for the metadata">> detail{""};

    GR_MAKE_REFLECTABLE(SigMFSink, in, file_name, sample_rate, author, detail);

    std::ofstream  _dataFile{};
    std::uint64_t  _sampleCount{0};

    std::expected<void, gr::Error> start() {
        const std::string base = static_cast<const std::string&>(file_name);
        _dataFile.open(base + ".sigmf-data", std::ios::binary | std::ios::trunc);
        if (!_dataFile.is_open()) {
            return std::unexpected(gr::Error{std::format("SigMFSink: cannot open '{}.sigmf-data'", base)});
        }
        _sampleCount = 0;
        return {};
    }

    std::expected<void, gr::Error> stop() {
        if (_dataFile.is_open()) _dataFile.close();
        return writeMetadata();
    }

    void processOne(T x) noexcept {
        if (!_dataFile.is_open()) return;
        _dataFile.write(reinterpret_cast<const char*>(&x), sizeof(T));
        ++_sampleCount;
    }

private:
    [[nodiscard]] std::expected<void, gr::Error> writeMetadata() const {
        const std::string base   = static_cast<const std::string&>(file_name);
        const std::string dtype  = std::string(detail::sigmfDatatype<T>());
        const double      sr     = static_cast<double>(sample_rate);
        const std::string auth   = static_cast<const std::string&>(author);
        const std::string desc   = static_cast<const std::string&>(detail);

        std::ofstream meta(base + ".sigmf-meta");
        if (!meta.is_open()) {
            return std::unexpected(gr::Error{std::format("SigMFSink: cannot open '{}.sigmf-meta'", base)});
        }

        meta << "{\n"
             << "  \"global\": {\n"
             << "    \"core:datatype\": \"" << dtype << "\",\n"
             << "    \"core:sample_rate\": " << sr << ",\n"
             << "    \"core:num_channels\": 1,\n"
             << "    \"core:version\": \"1.0.0\"";
        if (!auth.empty()) meta << ",\n    \"core:author\": \"" << auth << "\"";
        if (!desc.empty()) meta << ",\n    \"core:description\": \"" << desc << "\"";
        meta << "\n  },\n"
             << "  \"captures\": [\n"
             << "    {\"core:sample_start\": 0}\n"
             << "  ],\n"
             << "  \"annotations\": []\n"
             << "}\n";
        return {};
    }
};

} // namespace gr::blocks::fileio

#endif // GNURADIO_SIGMF_SINK_HPP
