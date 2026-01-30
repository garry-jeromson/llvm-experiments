/**
 * W65816 Integration Test Runner
 *
 * Executes W65816 binaries using the 816CE CPU emulator and reports results.
 * Test binaries should store their result at $0000-$0001 and execute STP.
 */

#include "../816ce/src/cpu/65816.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define MEM_SIZE    0x10000   // 64KB address space
#define ROM_START   0x8000    // Code loaded here
#define RESULT_ADDR 0x0000    // Test result stored here
#define MAX_CYCLES  10000000  // Cycle limit (10M)

static void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s [options] <binary>\n", prog);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -e, --expect <value>   Expected result value\n");
    fprintf(stderr, "  -v, --verbose          Verbose output\n");
    fprintf(stderr, "  -d, --debug            Debug output (show CPU state)\n");
    fprintf(stderr, "  -c, --cycles <limit>   Cycle limit (default: %d)\n", MAX_CYCLES);
    fprintf(stderr, "  -o, --org <addr>       Load address (default: 0x%04X)\n", ROM_START);
    fprintf(stderr, "  -h, --help             Show this help\n");
}

static void print_cpu_state(CPU_t *cpu) {
    printf("  A=%04X X=%04X Y=%04X SP=%04X D=%04X PC=%02X:%04X\n",
           cpu->C, cpu->X, cpu->Y, cpu->SP, cpu->D, cpu->PBR, cpu->PC);
    printf("  Flags: %c%c%c%c%c%c%c%c (E=%d)\n",
           cpu->P.N ? 'N' : 'n',
           cpu->P.V ? 'V' : 'v',
           cpu->P.M ? 'M' : 'm',
           cpu->P.XB ? 'X' : 'x',
           cpu->P.D ? 'D' : 'd',
           cpu->P.I ? 'I' : 'i',
           cpu->P.Z ? 'Z' : 'z',
           cpu->P.C ? 'C' : 'c',
           cpu->P.E);
}

int main(int argc, char **argv) {
    int expected = -1;
    int verbose = 0;
    int debug = 0;
    uint64_t cycle_limit = MAX_CYCLES;
    uint16_t load_addr = ROM_START;

    static struct option long_options[] = {
        {"expect",  required_argument, 0, 'e'},
        {"verbose", no_argument,       0, 'v'},
        {"debug",   no_argument,       0, 'd'},
        {"cycles",  required_argument, 0, 'c'},
        {"org",     required_argument, 0, 'o'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "e:vdc:o:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'e':
                expected = (int)strtol(optarg, NULL, 0);
                break;
            case 'v':
                verbose = 1;
                break;
            case 'd':
                debug = 1;
                verbose = 1;
                break;
            case 'c':
                cycle_limit = strtoull(optarg, NULL, 0);
                break;
            case 'o':
                load_addr = (uint16_t)strtol(optarg, NULL, 0);
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                return opt == 'h' ? 0 : 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: No binary file specified\n");
        print_usage(argv[0]);
        return 1;
    }

    const char *binary_file = argv[optind];

    // Allocate memory
    memory_t *mem = calloc(MEM_SIZE, sizeof(memory_t));
    if (!mem) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return 1;
    }

    // Load binary
    FILE *f = fopen(binary_file, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open '%s'\n", binary_file);
        free(mem);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size > MEM_SIZE - load_addr) {
        fprintf(stderr, "Error: Binary too large (%zu bytes)\n", size);
        fclose(f);
        free(mem);
        return 1;
    }

    for (size_t i = 0; i < size; i++) {
        int c = fgetc(f);
        if (c == EOF) break;
        mem[load_addr + i].val = (uint8_t)c;
    }
    fclose(f);

    if (verbose) {
        printf("Loaded %zu bytes at $%04X from '%s'\n", size, load_addr, binary_file);
    }

    // Set reset vector to load address
    mem[CPU_VEC_RESET].val = load_addr & 0xFF;
    mem[CPU_VEC_RESET + 1].val = (load_addr >> 8) & 0xFF;

    // Initialize CPU
    CPU_t cpu;
    CPU_Error_Code_t err = initCPU(&cpu);
    if (err != CPU_ERR_OK) {
        fprintf(stderr, "Error: Failed to initialize CPU\n");
        free(mem);
        return 1;
    }

    err = resetCPU(&cpu);
    if (err != CPU_ERR_OK) {
        fprintf(stderr, "Error: Failed to reset CPU\n");
        free(mem);
        return 1;
    }

    if (debug) {
        printf("Initial CPU state:\n");
        print_cpu_state(&cpu);
    }

    // Execute until STP/WDM or cycle limit
    int stopped = 0;
    const char *stop_reason = "timeout";
    uint64_t start_cycles = cpu.cycles;

    while (cpu.cycles - start_cycles < cycle_limit) {
        // Check for STP instruction (0xDB)
        if (cpu.P.STP) {
            stopped = 1;
            stop_reason = "STP";
            break;
        }

        // Check for WDM instruction (0x42) - used as alternative halt
        uint8_t opcode = mem[((uint32_t)cpu.PBR << 16) | cpu.PC].val;
        if (opcode == 0x42) {  // WDM
            stopped = 1;
            stop_reason = "WDM";
            break;
        }

        // Check for crash
        if (cpu.P.CRASH) {
            stop_reason = "CRASH";
            break;
        }

        err = stepCPU(&cpu, mem);

        if (debug && (cpu.cycles - start_cycles) < 100) {
            printf("Step %llu: PC=$%04X op=$%02X\n",
                   cpu.cycles - start_cycles,
                   cpu.PC,
                   mem[cpu.PC].val);
            print_cpu_state(&cpu);
        }

        if (err == CPU_ERR_STP) {
            stopped = 1;
            stop_reason = "STP";
            break;
        } else if (err != CPU_ERR_OK) {
            fprintf(stderr, "Error: CPU error %d at PC=$%04X\n", err, cpu.PC);
            stop_reason = "error";
            break;
        }
    }

    // Read result from memory (16-bit little-endian)
    uint16_t result = mem[RESULT_ADDR].val | (mem[RESULT_ADDR + 1].val << 8);

    // Report results
    uint64_t elapsed = cpu.cycles - start_cycles;

    if (verbose) {
        printf("Stopped: %s after %llu cycles\n", stop_reason, elapsed);
        printf("Final CPU state:\n");
        print_cpu_state(&cpu);
        printf("Memory[$0000]: $%04X (%d)\n", result, (int16_t)result);
    }

    // Determine pass/fail
    int exit_code = 0;

    if (!stopped) {
        if (!verbose) {
            printf("TIMEOUT after %llu cycles\n", elapsed);
        }
        exit_code = 2;
    } else if (expected >= 0) {
        if ((int)result == expected) {
            printf("PASS: result=%d (expected %d) [%llu cycles]\n",
                   result, expected, elapsed);
        } else {
            printf("FAIL: result=%d (expected %d) [%llu cycles]\n",
                   result, expected, elapsed);
            exit_code = 1;
        }
    } else {
        printf("Result: %d (0x%04X) [%llu cycles]\n", result, result, elapsed);
    }

    free(mem);
    return exit_code;
}
