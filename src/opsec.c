#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define OPSEC_VERSION "0.1.0"
#define DEFAULT_FRAME_DELAY_MS 70
#define FALLBACK_WIDTH 80
#define FALLBACK_HEIGHT 24

static volatile sig_atomic_t stop_requested = 0;
static bool cursor_hidden = false;

static const char *const train[] = {
    "          .----------------------------------------------------.",
    "  _______/  OPSEC EXPRESS // CHECK DOTFILES // NO LEAKS       \\___",
    " /  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  \\",
    "|  | CLASSIFIED | [REDACTED] | SHIP? NO. AUDIT. | 0xDEAD |  |",
    "'--o-------------o------------o------------------o---------o------'",
    "    O             O            O                  O         O     "
};

#define TRAIN_ROWS ((int)(sizeof(train) / sizeof(train[0])))

static void handle_signal(int signal_number)
{
    (void)signal_number;
    stop_requested = 1;
}

static void restore_terminal(void)
{
    if (!cursor_hidden) {
        return;
    }

    fputs("\033[?25h\033[0m\n", stdout);
    fflush(stdout);
    cursor_hidden = false;
}

static void sleep_for_ms(int milliseconds)
{
    struct timespec remaining = {
        .tv_sec = milliseconds / 1000,
        .tv_nsec = (long)(milliseconds % 1000) * 1000000L
    };

    while (!stop_requested && nanosleep(&remaining, &remaining) == -1 && errno == EINTR) {
        /* Continue sleeping unless a signal requested a clean exit. */
    }
}

static int terminal_dimensions(int *width, int *height)
{
    struct winsize window = {0};

    *width = FALLBACK_WIDTH;
    *height = FALLBACK_HEIGHT;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window) == -1) {
        return 0;
    }

    if (window.ws_col > 0) {
        *width = (int)window.ws_col;
    }
    if (window.ws_row > 0) {
        *height = (int)window.ws_row;
    }

    return 1;
}

static const char *row_color(int row)
{
    if (row == 0 || row == 1) {
        return "\033[1;33m";
    }
    if (row == 2 || row == 3) {
        return "\033[1;31m";
    }
    return "\033[1;37m";
}

static void render_line(const char *line, int x, int width, int row, bool color)
{
    int line_length = (int)strlen(line);
    int start = x < 0 ? -x : 0;
    int end = line_length;

    if (x + end > width) {
        end = width - x;
    }

    printf("\033[%d;1H\033[K", row + 1);

    if (start >= end || end <= 0) {
        return;
    }

    if (start < 0) {
        start = 0;
    }

    if (color) {
        fputs(row_color(row), stdout);
    }

    if (x > 0) {
        int padding = x < width ? x : width;
        for (int index = 0; index < padding; ++index) {
            putchar(' ');
        }
    }

    fwrite(line + start, 1, (size_t)(end - start), stdout);

    if (color) {
        fputs("\033[0m", stdout);
    }
}

static void render_frame(int x, int width, int height, bool color)
{
    int top = (height - TRAIN_ROWS) / 2;

    if (top < 0) {
        top = 0;
    }

    for (int row = 0; row < TRAIN_ROWS; ++row) {
        if (top + row >= height) {
            break;
        }
        render_line(train[row], x, width, top + row, color);
    }
    fflush(stdout);
}

static void render_plain_frame(void)
{
    for (int row = 0; row < TRAIN_ROWS; ++row) {
        puts(train[row]);
    }
}

static int train_width(void)
{
    int widest = 0;

    for (int row = 0; row < TRAIN_ROWS; ++row) {
        int current = (int)strlen(train[row]);
        if (current > widest) {
            widest = current;
        }
    }

    return widest;
}

static int parse_speed(const char *value, int *speed)
{
    char *end = NULL;
    long parsed;

    errno = 0;
    parsed = strtol(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0' || parsed < 10 || parsed > 2000) {
        return 0;
    }

    *speed = (int)parsed;
    return 1;
}

static void print_help(const char *program)
{
    printf("Usage: %s [OPTION]\n\n", program);
    puts("Run the OPSEC Express: a tiny terminal privacy meme for when you meant ls.");
    puts("");
    puts("Options:");
    puts("  -1, --once       print one frame and exit");
    puts("  -n, --no-color   disable ANSI colors");
    puts("  -s, --speed MS   set frame delay in milliseconds (10-2000)");
    puts("  -h, --help       show this help");
    puts("  -v, --version    show version");
    puts("");
    puts("Environment:");
    puts("  NO_COLOR         disable ANSI colors");
}

static int parse_arguments(int argc, char **argv, bool *once, bool *no_color, int *speed)
{
    for (int index = 1; index < argc; ++index) {
        const char *argument = argv[index];

        if (strcmp(argument, "-1") == 0 || strcmp(argument, "--once") == 0) {
            *once = true;
        } else if (strcmp(argument, "-n") == 0 || strcmp(argument, "--no-color") == 0) {
            *no_color = true;
        } else if (strcmp(argument, "-s") == 0 || strcmp(argument, "--speed") == 0) {
            if (index + 1 >= argc || !parse_speed(argv[++index], speed)) {
                fprintf(stderr, "opsec: --speed expects an integer from 10 to 2000\n");
                return 2;
            }
        } else if (strncmp(argument, "--speed=", 8) == 0) {
            if (!parse_speed(argument + 8, speed)) {
                fprintf(stderr, "opsec: --speed expects an integer from 10 to 2000\n");
                return 2;
            }
        } else if (strcmp(argument, "-h") == 0 || strcmp(argument, "--help") == 0) {
            print_help(argv[0]);
            return 1;
        } else if (strcmp(argument, "-v") == 0 || strcmp(argument, "--version") == 0) {
            printf("opsec %s\n", OPSEC_VERSION);
            return 1;
        } else {
            fprintf(stderr, "opsec: unknown option: %s\n", argument);
            fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
            return 2;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    bool once = false;
    bool no_color = false;
    bool interactive;
    int speed = DEFAULT_FRAME_DELAY_MS;
    int width;
    int height;
    int argument_status;

    argument_status = parse_arguments(argc, argv, &once, &no_color, &speed);
    if (argument_status != 0) {
        return argument_status == 1 ? EXIT_SUCCESS : argument_status;
    }

    interactive = isatty(STDOUT_FILENO) != 0 && !once;
    terminal_dimensions(&width, &height);

    if (!interactive) {
        render_plain_frame();
        return EXIT_SUCCESS;
    }

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    atexit(restore_terminal);

    fputs("\033[2J\033[H\033[?25l", stdout);
    cursor_hidden = true;
    fflush(stdout);

    int width_of_train = train_width();
    for (int x = -width_of_train; x <= width && !stop_requested; ++x) {
        render_frame(x, width, height, !no_color && getenv("NO_COLOR") == NULL);
        sleep_for_ms(speed);
    }

    return EXIT_SUCCESS;
}
