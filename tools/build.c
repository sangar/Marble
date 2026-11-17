// Builds and checks Marble. Bootstrap once with `cc -std=c23 -o nob tools/build.c`; after that
// ./nob rebuilds itself when this file changes.
//
//   ./nob [build]                      firmware for the gateway and a node into out/
//   ./nob test                         host tests with sanitizers, and the bridge's tests
//   ./nob check                        modern-c checker and the bridge's type check
//   ./nob ci                           build, test and check
//   ./nob flash gateway PORT           build and flash the gateway
//   ./nob flash node ID PORT           build and flash node ID (1-255)
//                                      BAUD=57600 for Nanos with the old bootloader
//   ./nob clean

#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

static const char *const homebrew_avr_gcc = "/opt/homebrew/opt/avr-gcc@15/bin/avr-gcc";

static bool run(const char *const argv[]) {
    for (const char *const *arg = argv; *arg; arg++) printf("%s%s", arg == argv ? "" : " ", *arg);
    printf("\n");
    fflush(stdout);

    pid_t pid;
    if (posix_spawnp(&pid, argv[0], nullptr, nullptr, (char *const *)argv, environ) != 0) {
        fprintf(stderr, "could not start %s\n", argv[0]);
        return false;
    }
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

static bool is_newer(const char *path, const char *than) {
    struct stat a, b;
    if (stat(path, &a) != 0) return false;
    if (stat(than, &b) != 0) return true;
    return a.st_mtimespec.tv_sec > b.st_mtimespec.tv_sec;
}

static void rebuild_self(char **argv) {
    if (!is_newer("tools/build.c", "nob")) return;
    if (!run((const char *[]){"cc", "-std=c23", "-Wall", "-Wextra", "-Werror", "-o", "nob", "tools/build.c", nullptr})) {
        exit(1);
    }
    execv("./nob", argv);
    perror("./nob");
    exit(1);
}

static const char *avr_gcc(void) {
    return access(homebrew_avr_gcc, X_OK) == 0 ? homebrew_avr_gcc : "avr-gcc";
}

static bool build_firmware(const char *program, const char *node_id, const char *name) {
    char source[64], elf[64], hex[64], define[32];
    snprintf(source, sizeof source, "src/%s.c", program);
    snprintf(elf, sizeof elf, "out/%s.elf", name);
    snprintf(hex, sizeof hex, "out/%s.hex", name);
    snprintf(define, sizeof define, "-DNODE_ID=%s", node_id);

    return run((const char *[]){avr_gcc(), "-std=c23", "-Os", "-mmcu=atmega328p", "-DF_CPU=16000000UL", define,
                                "-Wall", "-Wextra", "-Werror", "-Wconversion", "-Wshadow", "-Wvla",
                                "-Wstrict-prototypes", "-Wimplicit-fallthrough", "-Wno-unused-parameter",
                                "-ffunction-sections", "-fdata-sections", "-Wl,--gc-sections", "-Isrc",
                                "-o", elf, source, nullptr}) &&
           run((const char *[]){"avr-objcopy", "-O", "ihex", "-R", ".eeprom", elf, hex, nullptr}) &&
           run((const char *[]){"avr-size", elf, nullptr});
}

static bool build(void) {
    mkdir("out", 0755);
    return build_firmware("gateway", "0", "gateway") && build_firmware("node", "1", "node-1");
}

static bool bridge_installed(void) {
    return access("bridge/node_modules", F_OK) == 0 || run((const char *[]){"npm", "--prefix", "bridge", "ci", nullptr});
}

static bool test(void) {
    mkdir("out", 0755);
    return run((const char *[]){"cc", "-std=c23", "-g", "-O1", "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
                                "-Wall", "-Wextra", "-Werror", "-Wconversion", "-Wshadow", "-Wvla",
                                "-Wstrict-prototypes", "-Wimplicit-fallthrough", "-Wno-unused-parameter",
                                "-o", "out/test", "tests/test.c", nullptr}) &&
           run((const char *[]){"out/test", nullptr}) && bridge_installed() &&
           run((const char *[]){"npm", "--prefix", "bridge", "test", nullptr});
}

static bool check(void) {
    const char *home = getenv("HOME");
    char checker[512];
    snprintf(checker, sizeof checker, "%s/.agents/tools/modern-c/modern-c", home ? home : "");
    return run((const char *[]){"ruby", checker, "check", nullptr}) && bridge_installed() &&
           run((const char *[]){"npm", "--prefix", "bridge", "run", "check", nullptr});
}

static bool flash(int argc, char **argv) {
    bool gateway = argc == 4 && strcmp(argv[2], "gateway") == 0;
    bool node = argc == 5 && strcmp(argv[2], "node") == 0;
    if (!gateway && !node) {
        fprintf(stderr, "usage: ./nob flash gateway PORT | ./nob flash node ID PORT\n");
        return false;
    }
    if (node && (atoi(argv[3]) < 1 || atoi(argv[3]) > 255)) {
        fprintf(stderr, "node ID must be 1-255\n");
        return false;
    }

    char name[16], image[48];
    snprintf(name, sizeof name, gateway ? "gateway" : "node-%s", argv[3]);
    snprintf(image, sizeof image, "flash:w:out/%s.hex:i", name);
    const char *port = argv[argc - 1];
    const char *baud = getenv("BAUD") ? getenv("BAUD") : "115200";

    mkdir("out", 0755);
    return build_firmware(gateway ? "gateway" : "node", gateway ? "0" : argv[3], name) &&
           run((const char *[]){"avrdude", "-p", "atmega328p", "-c", "arduino", "-b", baud, "-P", port, "-U", image, nullptr});
}

int main(int argc, char **argv) {
    rebuild_self(argv);

    const char *command = argc > 1 ? argv[1] : "build";
    bool ok;
    if (strcmp(command, "build") == 0) {
        ok = build();
    } else if (strcmp(command, "test") == 0) {
        ok = test();
    } else if (strcmp(command, "check") == 0) {
        ok = check();
    } else if (strcmp(command, "ci") == 0) {
        ok = build() && test() && check();
    } else if (strcmp(command, "flash") == 0) {
        ok = flash(argc, argv);
    } else if (strcmp(command, "clean") == 0) {
        ok = run((const char *[]){"rm", "-rf", "out", nullptr});
    } else {
        fprintf(stderr, "unknown command: %s\n", command);
        ok = false;
    }
    if (ok && strcmp(command, "ci") == 0) printf("ci passed\n");
    return ok ? 0 : 1;
}
