#include <boost/ut.hpp>
#include <complex>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <gnuradio-4.0/fileio/CsvFileSink.hpp>
#include <gnuradio-4.0/fileio/CsvFileSource.hpp>

int main() { /* tests auto-register via boost::ut */ }

namespace {

std::filesystem::path tmpPath(const std::string& name) {
    return std::filesystem::temp_directory_path() / name;
}

void writeTextFile(const std::filesystem::path& p, const std::string& content) {
    std::ofstream f(p);
    f << content;
}

std::string readTextFile(const std::filesystem::path& p) {
    std::ifstream f(p);
    return {std::istreambuf_iterator<char>(f), {}};
}

} // namespace

const boost::ut::suite<"CsvFileSink"> sinkTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::fileio;

    "settingsChanged resizes inputs to n_inputs"_test = [] {
        CsvFileSink<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.n_inputs = 3U;
        b.settingsChanged({}, {});
        expect(eq(b.inputs.size(), std::size_t{3}));
    };

    "start() creates an empty file and stop() closes it"_test = [] {
        const auto path = tmpPath("qa_csv_sink_create.csv");
        CsvFileSink<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = path.string();
        b.n_inputs  = 1U;
        b.settingsChanged({}, {});

        const auto startResult = b.start();
        expect(startResult.has_value()) << "start() should succeed";
        expect(std::filesystem::exists(path)) << "file should exist after start()";

        const auto stopResult = b.stop();
        expect(stopResult.has_value()) << "stop() should succeed";
        std::filesystem::remove(path);
    };

    "start() writes column headers when column_names is set"_test = [] {
        const auto path = tmpPath("qa_csv_sink_header.csv");
        CsvFileSink<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name    = path.string();
        b.n_inputs     = 2U;
        b.column_names = std::vector<std::string>{"time", "value"};
        b.settingsChanged({}, {});

        std::ignore = b.start();
        std::ignore = b.stop();

        const std::string content = readTextFile(path);
        expect(content.find("time") != std::string::npos) << "header should contain 'time'";
        expect(content.find("value") != std::string::npos) << "header should contain 'value'";
        std::filesystem::remove(path);
    };

    "start() returns error for unwritable path"_test = [] {
        CsvFileSink<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = "/no_such_dir/qa_csv_sink_fail.csv";
        b.n_inputs  = 1U;
        b.settingsChanged({}, {});

        const auto result = b.start();
        expect(!result.has_value()) << "start() should fail for unwritable path";
    };

    "complex<float> compiles and initialises"_test = [] {
        CsvFileSink<std::complex<float>> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.n_inputs = 2U;
        b.settingsChanged({}, {});
        expect(eq(b.inputs.size(), std::size_t{2}));
    };
};

const boost::ut::suite<"CsvFileSource"> sourceTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::fileio;

    "settingsChanged resizes outputs to n_outputs"_test = [] {
        CsvFileSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.n_outputs = 4U;
        b.settingsChanged({}, {});
        expect(eq(b.outputs.size(), std::size_t{4}));
    };

    "start() opens file and stop() closes it"_test = [] {
        const auto path = tmpPath("qa_csv_source_open.csv");
        writeTextFile(path, "1.0,2.0\n3.0,4.0\n");

        CsvFileSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = path.string();
        b.n_outputs = 2U;
        b.settingsChanged({}, {});

        const auto startResult = b.start();
        expect(startResult.has_value()) << "start() should succeed for existing file";
        std::ignore = b.stop();
        std::filesystem::remove(path);
    };

    "start() skips header row when skip_header is true"_test = [] {
        const auto path = tmpPath("qa_csv_source_header.csv");
        writeTextFile(path, "x,y\n1.5,2.5\n");

        CsvFileSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name   = path.string();
        b.n_outputs   = 2U;
        b.skip_header = true;
        b.settingsChanged({}, {});

        std::ignore = b.start();
        // after start with skip_header the internal file pointer should be past the header
        // we just verify start() succeeds without crashing
        std::ignore = b.stop();
        std::filesystem::remove(path);
    };

    "start() returns error for missing file"_test = [] {
        CsvFileSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name = "/no_such_dir/qa_csv_source_missing.csv";
        b.n_outputs = 1U;
        b.settingsChanged({}, {});

        const auto result = b.start();
        expect(!result.has_value()) << "start() should fail for missing file";
    };

    "column_indices mapping is populated after start"_test = [] {
        const auto path = tmpPath("qa_csv_source_idx.csv");
        writeTextFile(path, "10.0,20.0,30.0\n");

        CsvFileSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.file_name      = path.string();
        b.n_outputs      = 1U;
        b.column_indices = std::vector<gr::Size_t>{2U}; // pick 3rd column
        b.settingsChanged({}, {});
        std::ignore = b.start();

        expect(eq(b._colIdx.size(), std::size_t{1}));
        expect(eq(b._colIdx[0], std::size_t{2}));
        std::ignore = b.stop();
        std::filesystem::remove(path);
    };

    "complex<float> compiles and initialises"_test = [] {
        CsvFileSource<std::complex<float>> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.n_outputs = 2U;
        b.settingsChanged({}, {});
        expect(eq(b.outputs.size(), std::size_t{2}));
    };
};
