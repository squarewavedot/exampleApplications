#include "Pacman.h"

#include <cstdlib>
#include <string>
#include <unistd.h>

int main(int argc, char *argv[]) {
    std::string serverUri = DEFAULTSERVERURI;
    int initialSeed = -1;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--seed" && i + 1 < argc) {
            initialSeed = std::atoi(argv[++i]);
        } else {
            serverUri = arg;
        }
    }

    Pacman app(serverUri, initialSeed);
    app.start();

    while (1) sleep(1);
    return 0;
}
