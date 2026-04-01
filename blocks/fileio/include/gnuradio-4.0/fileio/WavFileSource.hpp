#ifndef GNURADIO_WAV_FILE_SOURCE_HPP
#define GNURADIO_WAV_FILE_SOURCE_HPP

#include <bit>
#include <complex>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::fileio {

GR_REGISTER_BLOCK(gr::blocks::fileio::WavFileSource, [T], [ float, double, std::complex<float>, std::complex<double> ])

/**
 * @brief RIFF/WAV file source that reads PCM or IEEE-float samples into a typed stream.
 *
 * Parses the standard RIFF/WAVE header (fmt + data chunks) on start, then streams
 * samples. Mono files go to a real output; stereo files go to a complex output (channel 0
 * → real, channel 1 → imaginary). Supports int16, int32, and float32 PCM audio formats.
 * When `repeat` is true the file is rewound to the data start after the last sample.
 */
template<typename T>
struct WavFileSource : gr::Block<WavFileSource<T>> {
    using Description = Doc<R""(
@brief RIFF/WAV file source — reads PCM or IEEE-float samples into a typed stream.

Supports int16, int32, and float32 WAV audio. Mono files produce real samples;
stereo files produce complex samples (ch0→real, ch1→imaginary). When `repeat` is
true the file rewinds and restarts after the last sample; otherwise the block calls
`requestStop()` at EOF.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortOut<T> out;

    Annotated<std::string, "file_name", Doc<"input WAV file path">,             Visible> file_name{"input.wav"};
    Annotated<bool,        "repeat",    Doc<"rewind and replay from start at EOF">>       repeat{false};

    GR_MAKE_REFLECTABLE(WavFileSource, out, file_name, repeat);

    std::ifstream  _file{};
    std::uint16_t  _audioFormat{1};  // 1=PCM, 3=IEEE float
    std::uint16_t  _channels{1};
    std::uint32_t  _sampleRate{44100};
    std::uint16_t  _bitsPerSample{16};
    std::streampos _dataStart{};
    std::uint32_t  _dataSize{0};

    std::expected<void, gr::Error> start() {
        _file.open(static_cast<const std::string&>(file_name), std::ios::binary);
        if (!_file.is_open()) {
            return std::unexpected(gr::Error{std::format("WavFileSource: cannot open '{}'",
                                                          static_cast<const std::string&>(file_name))});
        }
        return parseHeader();
    }

    std::expected<void, gr::Error> stop() {
        if (_file.is_open()) _file.close();
        return {};
    }

    [[nodiscard]] T processOne() noexcept {
        if (!_file.is_open() || !_file.good()) return T{};

        T sample{};
        if (!readSample(sample)) {
            if (static_cast<bool>(repeat)) {
                _file.clear();
                _file.seekg(_dataStart);
                if (!readSample(sample)) return T{};
            } else {
                this->requestStop();
            }
        }
        return sample;
    }

private:
    template<typename U>
    [[nodiscard]] static U readLE(std::ifstream& f) {
        U v{};
        f.read(reinterpret_cast<char*>(&v), sizeof(U));
        return v;
    }

    [[nodiscard]] std::expected<void, gr::Error> parseHeader() {
        // RIFF chunk
        char riff[4];
        _file.read(riff, 4);
        if (std::string(riff, 4) != "RIFF") {
            return std::unexpected(gr::Error{"WavFileSource: not a RIFF file"});
        }
        std::ignore = readLE<std::uint32_t>(_file); // file size minus 8
        char wave[4];
        _file.read(wave, 4);
        if (std::string(wave, 4) != "WAVE") {
            return std::unexpected(gr::Error{"WavFileSource: WAVE marker missing"});
        }

        // scan for fmt and data chunks
        bool foundFmt  = false;
        bool foundData = false;
        while (_file.good() && !(foundFmt && foundData)) {
            char   id[4];
            _file.read(id, 4);
            const auto size = readLE<std::uint32_t>(_file);
            const std::string chunkId(id, 4);

            if (chunkId == "fmt ") {
                _audioFormat  = readLE<std::uint16_t>(_file);
                _channels     = readLE<std::uint16_t>(_file);
                _sampleRate   = readLE<std::uint32_t>(_file);
                std::ignore   = readLE<std::uint32_t>(_file); // byteRate
                std::ignore   = readLE<std::uint16_t>(_file); // blockAlign
                _bitsPerSample = readLE<std::uint16_t>(_file);
                if (size > 16) _file.seekg(static_cast<std::streamoff>(size - 16), std::ios::cur);
                foundFmt = true;
            } else if (chunkId == "data") {
                _dataSize  = size;
                _dataStart = _file.tellg();
                foundData  = true;
            } else {
                _file.seekg(static_cast<std::streamoff>(size), std::ios::cur);
            }
        }
        if (!foundFmt || !foundData) {
            return std::unexpected(gr::Error{"WavFileSource: missing fmt or data chunk"});
        }
        return {};
    }

    [[nodiscard]] bool readSample(T& sample) noexcept {
        const auto readChannel = [&](value_type& ch) -> bool {
            if (_audioFormat == 3) { // IEEE float
                if (_bitsPerSample == 32) {
                    float v{};
                    if (!_file.read(reinterpret_cast<char*>(&v), 4)) return false;
                    ch = static_cast<value_type>(v);
                } else { // 64-bit float (non-standard but handle it)
                    double v{};
                    if (!_file.read(reinterpret_cast<char*>(&v), 8)) return false;
                    ch = static_cast<value_type>(v);
                }
            } else { // PCM
                if (_bitsPerSample == 16) {
                    std::int16_t v{};
                    if (!_file.read(reinterpret_cast<char*>(&v), 2)) return false;
                    ch = static_cast<value_type>(v) / static_cast<value_type>(32768);
                } else { // int32
                    std::int32_t v{};
                    if (!_file.read(reinterpret_cast<char*>(&v), 4)) return false;
                    ch = static_cast<value_type>(v) / static_cast<value_type>(2147483648LL);
                }
            }
            return true;
        };

        value_type re{};
        if (!readChannel(re)) return false;

        if constexpr (gr::meta::complex_like<T>) {
            value_type im{};
            if (_channels >= 2) {
                if (!readChannel(im)) { im = value_type{}; }
            }
            sample = T{re, im};
        } else {
            // skip extra channels for mono output from stereo file
            for (std::uint16_t c = 1; c < _channels; ++c) {
                value_type skip{};
                std::ignore = readChannel(skip);
            }
            sample = static_cast<T>(re);
        }
        return _file.good() || _file.eof();
    }
};

} // namespace gr::blocks::fileio

#endif // GNURADIO_WAV_FILE_SOURCE_HPP
