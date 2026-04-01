#ifndef GNURADIO_WAV_FILE_SINK_HPP
#define GNURADIO_WAV_FILE_SINK_HPP

#include <complex>
#include <cstdint>
#include <fstream>
#include <string>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::fileio {

GR_REGISTER_BLOCK(gr::blocks::fileio::WavFileSink, [T], [ float, double, std::complex<float>, std::complex<double> ])

/**
 * @brief RIFF/WAV file sink that writes a typed sample stream to a standard WAV file.
 *
 * Writes a RIFF/WAVE file containing either 32-bit float (for real/complex<float>) or
 * 64-bit float (for double/complex<double>) samples. Stereo output is used for complex
 * inputs (real→ch0, imaginary→ch1). The RIFF and data chunk sizes are written as
 * placeholders on start() and fixed up by seeking back on stop().
 */
template<typename T>
struct WavFileSink : gr::Block<WavFileSink<T>> {
    using Description = Doc<R""(
@brief RIFF/WAV file sink — writes a typed sample stream to a WAV file.

Writes 32-bit IEEE float samples (for float/complex<float>) or 64-bit IEEE float
(for double/complex<double>). Complex inputs produce a stereo file (real→ch0,
imaginary→ch1). Chunk sizes are patched on `stop()`.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T> in;

    Annotated<std::string, "file_name",   Doc<"output WAV file path">,               Visible> file_name{"output.wav"};
    Annotated<double,      "sample_rate", Doc<"sample rate to embed in the WAV header">, Visible> sample_rate{44100.0};

    GR_MAKE_REFLECTABLE(WavFileSink, in, file_name, sample_rate);

    std::ofstream  _file{};
    std::uint32_t  _sampleCount{0};
    std::streampos _riffSizePos{};
    std::streampos _dataSizePos{};

    std::expected<void, gr::Error> start() {
        _sampleCount = 0;
        _file.open(static_cast<const std::string&>(file_name), std::ios::binary | std::ios::trunc);
        if (!_file.is_open()) {
            return std::unexpected(gr::Error{std::format("WavFileSink: cannot open '{}' for writing",
                                                          static_cast<const std::string&>(file_name))});
        }
        writeHeader();
        return {};
    }

    std::expected<void, gr::Error> stop() {
        if (!_file.is_open()) return {};
        patchSizes();
        _file.close();
        return {};
    }

    void processOne(T x) noexcept {
        if (!_file.is_open()) return;
        if constexpr (gr::meta::complex_like<T>) {
            writeSample(static_cast<value_type>(std::real(x)));
            writeSample(static_cast<value_type>(std::imag(x)));
        } else {
            writeSample(static_cast<value_type>(x));
        }
        ++_sampleCount;
    }

private:
    static constexpr std::uint16_t kChannels      = gr::meta::complex_like<T> ? 2U : 1U;
    static constexpr std::uint16_t kBitsPerSample = sizeof(value_type) == 4 ? 32U : 64U;
    static constexpr std::uint16_t kAudioFormat   = 3U; // IEEE float

    template<typename U>
    void writeLE(U v) {
        _file.write(reinterpret_cast<const char*>(&v), sizeof(U));
    }

    void writeHeader() {
        // RIFF chunk
        _file.write("RIFF", 4);
        _riffSizePos = _file.tellp();
        writeLE(std::uint32_t{0}); // placeholder: total file size - 8
        _file.write("WAVE", 4);

        // fmt chunk
        _file.write("fmt ", 4);
        writeLE(std::uint32_t{16}); // fmt chunk size
        writeLE(kAudioFormat);
        writeLE(kChannels);
        const auto sr      = static_cast<std::uint32_t>(sample_rate);
        const auto bAlign  = static_cast<std::uint16_t>(kChannels * kBitsPerSample / 8U);
        const auto byteRate = static_cast<std::uint32_t>(sr * bAlign);
        writeLE(sr);
        writeLE(byteRate);
        writeLE(bAlign);
        writeLE(kBitsPerSample);

        // data chunk header
        _file.write("data", 4);
        _dataSizePos = _file.tellp();
        writeLE(std::uint32_t{0}); // placeholder: data bytes
    }

    void writeSample(value_type v) {
        if constexpr (sizeof(value_type) == 4) {
            const float fv = static_cast<float>(v);
            _file.write(reinterpret_cast<const char*>(&fv), 4);
        } else {
            const double dv = static_cast<double>(v);
            _file.write(reinterpret_cast<const char*>(&dv), 8);
        }
    }

    void patchSizes() {
        const std::uint32_t bytesPerSample = kChannels * kBitsPerSample / 8U;
        const std::uint32_t dataBytes      = _sampleCount * bytesPerSample;
        // riff size = 4 (WAVE) + 8 (fmt id+size) + 16 (fmt data) + 8 (data id+size) + dataBytes
        const std::uint32_t riffSize       = 4U + 8U + 16U + 8U + dataBytes;

        _file.seekp(_riffSizePos);
        writeLE(riffSize);
        _file.seekp(_dataSizePos);
        writeLE(dataBytes);
    }
};

} // namespace gr::blocks::fileio

#endif // GNURADIO_WAV_FILE_SINK_HPP
