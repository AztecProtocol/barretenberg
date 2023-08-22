#pragma once
#include <cvc5/cvc5.h>
#include <string>

namespace smt_solver {

class Solver {
  private:
    bool res = false;
    bool checked = false;

  public:
    cvc5::Solver s;
    cvc5::Sort fp;

    // TODO(alex): more options???
    explicit Solver(const std::string& modulus, bool produce_model = false, uint32_t base = 16, uint32_t timeout = 0)
    {
        fp = s.mkFiniteFieldSort(modulus, base);
        if (produce_model) {
            s.setOption("produce-models", "true");
        }
        if (timeout > 0) {
            s.setOption("tlimit-per", std::to_string(timeout));
        }
    }

    Solver(const Solver& other) = delete;
    Solver(Solver&& other) = delete;
    Solver& operator=(const Solver& other) = delete;
    Solver& operator=(Solver&& other) = delete;

    bool check();

    [[nodiscard]] std::string getResult() const
    {
        if (!checked) {
            return "no result, yet";
        }
        return res ? "SAT" : "UNSAT";
    }

    std::unordered_map<std::string, std::string> model(std::unordered_map<std::string, cvc5::Term>& terms) const;
    ~Solver() = default;
};

}; // namespace smt_solver