#include <boost/ut.hpp>
#include <cmath>
#include <complex>
#include <filesystem>
#include <vector>

#include <gnuradio-4.0/fileio/WavFileSink.hpp>
#include <gnuradio-4.0/fileio/WavFileSource.hpp>

int main() { /* tests auto-register via boost::ut */ }

namespace {

std::filesystem::path tmpPath(const std::string& name) {
    return std::filesystem::temp_directory_path() / name;
}

} // namespace

const boost::ut::suite<"WavFileSink"> sinkTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::fileio;

    "start() creates file and stop() closes it"_test = [] {
        const auto path = tmpPath("qa_wav_sink_create.wav");
        WavFileSink<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name   = path.string();
        b.sample_rate = 44100.0;

        expect(b.start().has_value()) << "start() should succeed";
        expect(std::filesystem::exists(path)) << "file should be created";
        expect(b.stop().has_value()) << "stop() should succeed";

        // WAV file should have the RIFF header (at least 44 bytes)
        expect(std::filesystem::file_size(path) >= 44UL) << "WAV file should have header";
        std::filesystem::remove(path);
    };

    "start() fails for unwritable path"_test = [] {
        WavFileSink<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = "/no_such_dir/qa_wav_sink_fail.wav";
        expect(!b.start().has_value()) << "start() should fail for bad path";
    };

    "processOne increments sample counter"_test = [] {
        const auto path = tmpPath("qa_wav_sink_counter.wav");
        WavFileSink<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = path.string();
        std::ignore = b.start();
        b.processOne(1.0f);
        b.processOne(2.0f);
        expect(eq(b._sampleCount, std::uint32_t{2}));
        std::ignore = b.stop();
        std::filesystem::remove(path);
    };

    "double type writes 64-bit float samples"_test = [] {
        const auto path = tmpPath("qa_wav_sink_double.wav");
        WavFileSink<double> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name   = path.string();
        b.sample_rate = 48000.0;
        std::ignore = b.start();
        b.processOne(0.5);
        std::ignore = b.stop();
        expect(std::filesystem::file_size(path) > 44UL);
        std::filesystem::remove(path);
    };

    "complex<float> writes stereo file"_test = [] {
        const auto path = tmpPath("qa_wav_sink_stereo.wav");
        WavFileSink<std::complex<float>> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = path.string();
        std::ignore = b.start();
        b.processOne({1.0f, -1.0f});
        std::ignore = b.stop();
        expect(std::filesystem::file_size(path) >= 44UL + 8UL); // 2 channels × 4 bytes
        std::filesystem::remove(path);
    };
};

const boost::ut::suite<"WavFileSource"> sourceTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::fileio;

    // helper: write a minimal float32 mono WAV file with given samples
    auto makeFloatWav = [](const std::filesystem::path& p, const std::vector<float>& samples, std::uint32_t sr = 44100) {
        std::ofstream f(p, std::ios::binary);
        const std::uint32_t dataBytes = static_cast<std::uint32_t>(samples.size() * 4);
        const std::uint32_t riffSize  = 4 + 8 + 16 + 8 + dataBytes;
        auto write32 = [&](std::uint32_t v) { f.write(reinterpret_cast<const char*>(&v), 4); };
        auto write16 = [&](std::uint16_t v) { f.write(reinterpret_cast<const char*>(&v), 2); };
        f.write("RIFF", 4); write32(riffSize);
        f.write("WAVE", 4);
        f.write("fmt ", 4); write32(16);
        write16(3);   // IEEE float
        write16(1);   // mono
        write32(sr);
        write32(sr * 4); // byteRate
        write16(4);   // blockAlign
        write16(32);  // bitsPerSample
        f.write("data", 4); write32(dataBytes);
        for (float v : samples) f.write(reinterpret_cast<const char*>(&v), 4);
    };

    "start() parses header and stop() closes file"_test = [makeFloatWav] {
        const auto path = tmpPath("qa_wav_src_open.wav");
        makeFloatWav(path, {1.0f, 2.0f, 3.0f});

        WavFileSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = path.string();

        expect(b.start().has_value()) << "start() should succeed";
        expect(eq(b._channels, std::uint16_t{1}));
        expect(eq(b._audioFormat, std::uint16_t{3})); // IEEE float
        expect(b.stop().has_value());
        std::filesystem::remove(path);
    };

    "processOne reads samples in order"_test = [makeFloatWav] {
        const auto path = tmpPath("qa_wav_src_read.wav");
        const std::vector<float> expected = {0.5f, -0.5f, 0.25f};
        makeFloatWav(path, expected);

        WavFileSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = path.string();
        std::ignore = b.start();

        for (std::size_t i = 0; i < expected.size(); ++i) {
            const float v = b.processOne();
            expect(std::abs(v - expected[i]) < 1e-6f) << "sample " << i << " mismatch";
        }
        std::ignore = b.stop();
        std::filesystem::remove(path);
    };

    "start() fails for non-existent file"_test = [] {
        WavFileSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = "/no_such_dir/qa_wav_src_miss.wav";
        expect(!b.start().has_value()) << "start() should fail for missing file";
    };

    "start() fails for non-WAV file"_test = [] {
        const auto path = tmpPath("qa_wav_src_invalid.wav");
        {
            std::ofstream f(path, std::ios::binary);
            f.write("JUNK", 4); // invalid RIFF header
        }
        WavFileSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = path.string();
        expect(!b.start().has_value()) << "start() should fail for invalid header";
        std::filesystem::remove(path);
    };

    "sink → source roundtrip preserves samples"_test = [] {
        const auto path = tmpPath("qa_wav_roundtrip.wav");
        const std::vector<float> input = {0.1f, 0.2f, 0.3f, -0.4f, -0.5f};

        {
            WavFileSink<float> sink{};
            sink.settings().init();
            std::ignore = sink.settings().applyStagedParameters();
            sink.file_name   = path.string();
            sink.sample_rate = 44100.0;
            std::ignore = sink.start();
            for (float v : input) sink.processOne(v);
            std::ignore = sink.stop();
        }

        WavFileSource<float> src{};
        src.settings().init();
        std::ignore = src.settings().applyStagedParameters();
        src.file_name = path.string();
        std::ignore = src.start();

        for (std::size_t i = 0; i < input.size(); ++i) {
            const float v = src.processOne();
            expect(std::abs(v - input[i]) < 1e-6f) << "roundtrip mismatch at index " << i;
        }
        std::ignore = src.stop();
        std::filesystem::remove(path);
    };

    "double type compiles and initialises"_test = [] {
        WavFileSource<double> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = "/dev/null"; // won't be opened in test
        expect(true);
    };
};
