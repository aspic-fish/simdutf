#include "benchmark_base.h"
#include "tests/helpers/random_utf8.h"

#include <fstream>

namespace simdutf::benchmarks {

    BenchmarkBase::BenchmarkBase(std::vector<input::Testcase>&& testcases)
        : testcases{std::move(testcases)} {}

    bool BenchmarkBase::run() {
        printf("testcases: %lu\n", testcases.size());
        for (const auto& testcase: testcases) {
            run(testcase);
        }

        return true;
    }

    void BenchmarkBase::run(const input::Testcase& testcase) {
        prepare_input(testcase);

        const auto& known_procedurs = all_procedures();

        if (testcase.tested_procedures.empty()) {
            for (const auto& procedure: known_procedurs)
                run(procedure, testcase.iterations);
        } else {
            for (const auto& procedure: testcase.tested_procedures) {
                if (known_procedurs.count(procedure) > 0)
                    run(procedure, testcase.iterations);
            }
        }
    }

    void BenchmarkBase::prepare_input(const input::Testcase& testcase) {
        if (std::holds_alternative<input::File>(testcase.input)) {
            const input::File& file{std::get<input::File>(testcase.input)};
            load_file(file.path);
        } else {
            const input::RandomUTF8& random{std::get<input::RandomUTF8>(testcase.input)};

            simdutf::tests::helpers::RandomUTF8 gen_1_2_3_4(rd,
                random.utf_1byte_prob,
                random.utf_2bytes_prob,
                random.utf_3bytes_prob,
                random.utf_4bytes_prob);

            input_data = gen_1_2_3_4.generate(random.size);
        }
    }

    void BenchmarkBase::load_file(const std::filesystem::path& path) {
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        file.open(path);

        input_data.assign(std::istreambuf_iterator<char>(file),
                          std::istreambuf_iterator<char>());
    }

    void BenchmarkBase::print_summary(const event_aggregate& all, double data_size) const {
        const double _1GHz = 1'000'000'000.0;

        const double freq = (all.best.cycles() / all.best.elapsed_sec()) / _1GHz;
        const double insperunit = all.best.instructions() / data_size;
        const double gbs = data_size / all.best.elapsed_ns();

        const bool has_events = true; // TODO
        if (has_events) {
            printf("%8.3f ins/byte, %8.3f GHz, %8.3f GB/s \n", insperunit, freq, gbs);
        } else {
            printf("%8.3f GB/s \n", gbs);
        }
    }
}
