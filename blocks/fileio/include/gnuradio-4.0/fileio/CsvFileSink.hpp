#ifndef GNURADIO_CSV_FILE_SINK_HPP
#define GNURADIO_CSV_FILE_SINK_HPP

#include <cmath>
#include <complex>
#include <fstream>
#include <string>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::fileio {

GR_REGISTER_BLOCK(gr::blocks::fileio::CsvFileSink, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct CsvFileSink : gr::Block<CsvFileSink<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Writes one or more typed sample streams to a CSV file.

Each row in the output file contains one value per input port, separated by
`separator`. An optional header row is written at the start using `column_names`.
For complex inputs each sample writes two columns (real and imaginary).
The file is opened on `start()` and closed on `stop()`.
)"">;

    std::vector<PortIn<T>> inputs{};

    Annotated<std::string,              "file_name",    Doc<"output file path">,                      Visible> file_name{"output.csv"};
    Annotated<std::vector<std::string>, "column_names", Doc<"optional header names (empty = no header)">>      column_names{};
    Annotated<std::string,              "separator",    Doc<"column separator character (default ',')">>        separator{","};
    Annotated<gr::Size_t,               "n_inputs",     Doc<"number of input ports">,                 Visible> n_inputs{1U};

    GR_MAKE_REFLECTABLE(CsvFileSink, inputs, file_name, column_names, separator, n_inputs);

    std::ofstream _file{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        inputs.resize(static_cast<std::size_t>(n_inputs));
        this->input_chunk_size  = static_cast<gr::Size_t>(1);
        this->output_chunk_size = static_cast<gr::Size_t>(1);
    }

    std::expected<void, gr::Error> start() {
        _file.open(static_cast<const std::string&>(file_name), std::ios::out | std::ios::trunc);
        if (!_file.is_open()) {
            return std::unexpected(gr::Error{std::format("CsvFileSink: cannot open '{}' for writing",
                                                          static_cast<const std::string&>(file_name))});
        }
        const auto&       names = static_cast<const std::vector<std::string>&>(column_names);
        const std::string sep   = static_cast<const std::string&>(separator);
        if (!names.empty()) {
            bool first = true;
            for (const auto& name : names) {
                if (!first) _file << sep;
                first = false;
                _file << name;
            }
            _file << '\n';
        }
        return {};
    }

    std::expected<void, gr::Error> stop() {
        if (_file.is_open()) _file.close();
        return {};
    }

    template<gr::InputSpanLike TInSpan>
    [[nodiscard]] gr::work::Status processBulk(std::span<TInSpan>& ins) noexcept {
        if (!_file.is_open()) return gr::work::Status::ERROR;
        const std::string sep   = static_cast<const std::string&>(separator);
        bool              first = true;
        for (auto& col : ins) {
            if (!first) _file << sep;
            first = false;
            if constexpr (gr::meta::complex_like<T>) {
                _file << col[0].real() << sep << col[0].imag();
            } else {
                _file << col[0];
            }
        }
        _file << '\n';
        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::fileio

#endif // GNURADIO_CSV_FILE_SINK_HPP
