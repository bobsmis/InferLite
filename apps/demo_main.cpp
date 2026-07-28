#include <iostream>
#include "inferlite/core/version.h"

int main() {
    std::cout << inferlite::project_name() << '\n';

    return 0;
}