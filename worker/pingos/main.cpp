#include <cstdlib> // getenv

int MasterMain(int argc, char **argv);
int WorkerMain(int argc, char **argv);

int main(int argc, char **argv)
{
    if (!std::getenv("MEDIASOUP_VERSION")) {
        return MasterMain(argc, argv);
    } else {
        return WorkerMain(argc, argv);
    }

    return 0;
}
