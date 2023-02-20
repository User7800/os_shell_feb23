#include <iostream>
#include <getopt.h>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

using std::vector;
using std::string;
using std::cout;

#define DEBUG_MODE 1

int MAX_CMD_LEN = 1024;
int MAX_HIST_SIZE = 10;

int execute(string cmd, int foreground = 0) {
    int pid;
    pid = fork();
    if
    system(cmd.c_str());
    if(foreground) wait();

}

int main(int argc, char* argv[]) {

    int opt;
    while ((opt = getopt(argc, argv, "h:l:")) != -1) {
        switch (opt) {

            case 'h':
                MAX_HIST_SIZE = atoi(optarg);
                if(DEBUG_MODE) cout << "Option -h given with argument " << optarg << '\n';
                break;
            case 'l':
                MAX_CMD_LEN = atoi(optarg);
                if(DEBUG_MODE) cout << "Option -l given with argument " << optarg << '\n';
                break;
            case '?':
                if (optopt == 'h' || optopt == 's') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } 
                return 1;
            default:
                abort();
        }
    }



    return 0;
}