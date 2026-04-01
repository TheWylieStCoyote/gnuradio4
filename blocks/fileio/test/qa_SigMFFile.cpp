#include <boost/ut.hpp>
#include <complex>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gnuradio-4.0/fileio/SigMFSink.hpp>
#include <gnuradio-4.0/fileio/SigMFSource.hpp>

int main() { /* tests auto-register via boost::ut */ }

namespace {

std::filesystem::path tmpBase(const std::string& stem) {
    return std::filesystem::temp_directory_path() / stem;
}

std::string readFile(const std::filesystem::path& p) {
    std::ifstream f(p);
    return {std::istreambuf_iterator<char>(f), {}};
}

} // namespace

const boost::ut::suite<"SigMFSink"> sinkTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::fileio;

    "stop() writes .sigmf-meta with correct datatype for float"_test = [] {
        const auto base = tmpBase("qa_sigmf_sink_float").string();
        SigMFSink<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name   = base;
        b.sample_rate = 48000.0;
        std::ignore = b.start();
        std::ignore = b.stop();

        const std::string meta = readFile(base + ".sigmf-meta");
        expect(meta.find("rf32_le") != std::string::npos) << "datatype should be rf32_le";
        expect(meta.find("48000") != std::string::npos) << "sample rate should appear";
        std::filesystem::remove(base + ".sigmf-data");
        std::filesystem::remove(base + ".sigmf-meta");
    };

    "stop() writes .sigmf-meta with cf32_le for complex<float>"_test = [] {
        const auto base = tmpBase("qa_sigmf_sink_cf32").string();
        SigMFSink<std::complex<float>> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = base;
        std::ignore = b.start();
        std::ignore = b.stop();

        const std::string meta = readFile(base + ".sigmf-meta");
        expect(meta.find("cf32_le") != std::string::npos) << "datatype should be cf32_le";
        std::filesystem::remove(base + ".sigmf-data");
        std::filesystem::remove(base + ".sigmf-meta");
    };

    "processOne increments sample counter"_test = [] {
        const auto base = tmpBase("qa_sigmf_sink_cnt").string();
        SigMFSink<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = base;
        std::ignore = b.start();
        b.processOne(1.0f);
        b.processOne(2.0f);
        expect(eq(b._sampleCount, std::uint64_t{2}));
        std::ignore = b.stop();
        std::filesystem::remove(base + ".sigmf-data");
        std::filesystem::remove(base + ".sigmf-meta");
    };

    "start() fails for bad path"_test = [] {
        SigMFSink<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = "/no_such_dir/qa_sigmf_fail";
        expect(!b.start().has_value()) << "start() should fail for invalid path";
    };

    "double type compiles and generates rf64_le datatype"_test = [] {
        const auto base = tmpBase("qa_sigmf_sink_double").string();
        SigMFSink<double> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = base;
        std::ignore = b.start();
        std::ignore = b.stop();
        const std::string meta = readFile(base + ".sigmf-meta");
        expect(meta.find("rf64_le") != std::string::npos) << "datatype should be rf64_le";
        std::filesystem::remove(base + ".sigmf-data");
        std::filesystem::remove(base + ".sigmf-meta");
    };
};

const boost::ut::suite<"SigMFSource"> sourceTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::fileio;

    "start() opens .sigmf-data and stop() closes it"_test = [] {
        const auto base = tmpBase("qa_sigmf_src_open").string();
        {
            std::ofstream f(base + ".sigmf-data", std::ios::binary);
            const float v = 3.14f;
            f.write(reinterpret_cast<const char*>(&v), sizeof(v));
        }
        SigMFSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = base;
        expect(b.start().has_value()) << "start() should succeed";
        std::ignore = b.stop();
        std::filesystem::remove(base + ".sigmf-data");
    };

    "start() reads sample_rate from .sigmf-meta when present"_test = [] {
        const auto base = tmpBase("qa_sigmf_src_sr").string();
        {
            std::ofstream d(base + ".sigmf-data", std::ios::binary);
            const float v = 1.0f;
            d.write(reinterpret_cast<const char*>(&v), sizeof(v));
        }
        {
            std::ofstream m(base + ".sigmf-meta");
            m << R"({"global":{"core:datatype":"rf32_le","core:sample_rate":22050.0},"captures":[],"annotations":[]})";
        }
        SigMFSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = base;
        std::ignore = b.start();
        expect(std::abs(b.parsedSampleRate() - 22050.0) < 1.0) << "sample rate should be parsed from meta";
        std::ignore = b.stop();
        std::filesystem::remove(base + ".sigmf-data");
        std::filesystem::remove(base + ".sigmf-meta");
    };

    "start() fails for missing file"_test = [] {
        SigMFSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = "/no_such_dir/qa_sigmf_miss";
        expect(!b.start().has_value()) << "start() should fail";
    };

    "sink → source roundtrip preserves samples"_test = [] {
        const auto base = tmpBase("qa_sigmf_roundtrip").string();
        const std::vector<float> input = {0.1f, -0.2f, 0.3f, -0.4f};
        {
            SigMFSink<float> sink{};
            sink.settings().init();
            std::ignore = sink.settings().applyStagedParameters();
            sink.file_name   = base;
            sink.sample_rate = 44100.0;
            std::ignore = sink.start();
            for (float v : input) sink.processOne(v);
            std::ignore = sink.stop();
        }
        SigMFSource<float> src{};
        src.settings().init();
        std::ignore = src.settings().applyStagedParameters();
        src.file_name = base;
        std::ignore = src.start();
        for (std::size_t i = 0; i < input.size(); ++i) {
            const float v = src.processOne();
            expect(std::abs(v - input[i]) < 1e-6f) << "roundtrip mismatch at " << i;
        }
        std::ignore = src.stop();
        std::filesystem::remove(base + ".sigmf-data");
        std::filesystem::remove(base + ".sigmf-meta");
    };
};
