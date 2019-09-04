#include "picture.h"

int main(int argc, char *argv[]) {
    Picture App1(argc, argv);
    App1.start();

    while(1) sleep(2);
    return 0;
}
