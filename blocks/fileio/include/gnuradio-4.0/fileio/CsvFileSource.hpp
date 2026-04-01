#ifndef GNURADIO_CSV_FILE_SOURCE_HPP
#define GNURADIO_CSV_FILE_SOURCE_HPP

#include <complex>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::fileio {

GR_REGISTER_BLOCK(gr::blocks::fileio::CsvFileSource, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct CsvFileSource : gr::Block<CsvFileSource<T>> {
    using Description = Doc<R""(
@brief Reads a CSV file and streams one value per output port per row.

Each output port receives one column from the file. `column_indices` selects which
CSV columns (0-based) map to each output port; if empty, the first `n_outputs`
columns are used. When `skip_header` is true the first row is discarded. Complex
outputs require two consecutive CSV columns (real, imaginary). The block calls
`requestStop()` when the file is exhausted.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    std::vector<PortOut<T>> outputs{};

    Annotated<std::string,           "file_name",      Doc<"input CSV file path">,                                     Visible> file_name{"input.csv"};
    Annotated<std::vector<gr::Size_t>,"column_indices", Doc<"CSV column indices per output (empty = first n_outputs)">>          column_indices{};
    Annotated<std::string,           "separator",      Doc<"column separator character (default ',')">>                          separator{","};
    Annotated<bool,                  "skip_header",    Doc<"skip the first row of the CSV file">>                                skip_header{false};
    Annotated<gr::Size_t,            "n_outputs",      Doc<"number of output ports">,                                  Visible> n_outputs{1U};

    GR_MAKE_REFLECTABLE(CsvFileSource, outputs, file_name, column_indices, separator, skip_header, n_outputs);

    std::ifstream              _file{};
    std::vector<std::size_t>   _colIdx{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        outputs.resize(static_cast<std::size_t>(n_outputs));
    }

    std::expected<void, gr::Error> start() {
        _file.open(static_cast<const std::string&>(file_name));
        if (!_file.is_open()) {
            return std::unexpected(gr::Error{std::format("CsvFileSource: cannot open '{}'",
                                                          static_cast<const std::string&>(file_name))});
        }
        if (static_cast<bool>(skip_header)) {
            std::string line;
            std::getline(_file, line);
        }
        // build column index mapping
        const auto& userIdx = static_cast<const std::vector<gr::Size_t>&>(column_indices);
        const std::size_t n = static_cast<std::size_t>(n_outputs);
        _colIdx.resize(n);
        for (std::size_t i = 0; i < n; ++i) {
            _colIdx[i] = (i < userIdx.size()) ? static_cast<std::size_t>(userIdx[i]) : i;
        }
        return {};
    }

    std::expected<void, gr::Error> stop() {
        if (_file.is_open()) _file.close();
        return {};
    }

    template<gr::OutputSpanLike TOutSpan>
    [[nodiscard]] gr::work::Status processBulk(std::span<TOutSpan>& outs) noexcept {
        if (!_file.is_open() || outs.empty()) return gr::work::Status::ERROR;

        const char        sep      = static_cast<const std::string&>(separator).empty()
                                         ? ',' : static_cast<const std::string&>(separator)[0];
        const std::size_t nSamples = outs[0].size();
        std::size_t       s        = 0;

        for (; s < nSamples; ++s) {
            std::string line;
            if (!std::getline(_file, line)) {
                this->requestStop();
                break;
            }
            std::vector<std::string> tokens;
            std::istringstream       ss{line};
            std::string              tok;
            while (std::getline(ss, tok, sep)) tokens.push_back(std::move(tok));

            for (std::size_t i = 0; i < outs.size(); ++i) {
                const std::size_t col = _colIdx[i];
                if constexpr (gr::meta::complex_like<T>) {
                    const value_type re = (col     < tokens.size()) ? static_cast<value_type>(std::stod(tokens[col]))     : value_type{};
                    const value_type im = (col + 1 < tokens.size()) ? static_cast<value_type>(std::stod(tokens[col + 1])) : value_type{};
                    outs[i][s] = T{re, im};
                } else {
                    outs[i][s] = (col < tokens.size()) ? static_cast<T>(std::stod(tokens[col])) : T{};
                }
            }
        }
        for (auto& out : outs) out.publish(s);
        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::fileio

#endif // GNURADIO_CSV_FILE_SOURCE_HPP
