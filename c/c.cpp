#include <iostream>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <evaluate at x> [coefficients...]" << std::endl;
        return 1;
    }

    int x = atoi(argv[1]);
    int num_terms = argc - 2;
    std::vector<int> coefficients(num_terms);
    for (int i = 0; i < num_terms; ++i) {
        coefficients[i] = atoi(argv[num_terms - i + 1]);
    }

    std::vector<int> fds(num_terms * 2);

    for (int i = 0; i < num_terms; ++i) {
        if (pipe(&fds[2 * i]) == -1) {
            std::cerr << "Failed to create pipe" << std::endl;
            return 1;
        }

        pid_t pid = fork();
        if (pid == -1) {
            std::cerr << "Failed to fork" << std::endl;
            return 1;
        }

        if (pid == 0) { // Child process
            close(fds[2 * i]);
            int power = num_terms - 1 - i;
            int result = coefficients[i] * std::pow(x, power);
            if (write(fds[2 * i + 1], &result, sizeof(result)) != sizeof(result)) {
                std::cerr << "Failed to write result" << std::endl;
                exit(1);
            }
            close(fds[2 * i + 1]);
            exit(0);
        } else { // Parent process
            close(fds[2 * i + 1]);
        }
    }

    int final_result = 0;
    int term_result;
    for (int i = 0; i < num_terms; ++i) {
        if (read(fds[2 * i], &term_result, sizeof(term_result)) != sizeof(term_result)) {
            std::cerr << "Failed to read result" << std::endl;
        }
        final_result += term_result;
        close(fds[2 * i]);
    }

    while (wait(NULL) > 0);

    std::cout << "Polynomial result: " << final_result << std::endl;

    return 0;
}
