#include "integration_class.hpp"

std::ofstream test_log("integration_test.log");



int main(int argc, char** argv) {

    IntegrationTest t(1, 1);
    t.Run();
    return 0;
}
