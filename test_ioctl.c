#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

/* ioctl commands - must match morse_dev.c */
#define MORSE_SET_UNIT_DURATION _IOW('m', 1, int)
#define MORSE_GET_UNIT_DURATION _IOR('m', 2, int)
#define MORSE_SET_BUFFER_SIZE   _IOW('m', 3, int)
#define MORSE_GET_BUFFER_SIZE   _IOR('m', 4, int)

void print_usage(char *prog) {
    printf("Usage:\n");
    printf("  %s get_duration        - Get current unit duration\n", prog);
    printf("  %s set_duration <ms>   - Set unit duration (10-1000ms)\n", prog);
    printf("  %s get_size            - Get current buffer size\n", prog);
    printf("  %s set_size <bytes>    - Set buffer size (64-4096 bytes)\n", prog);
    printf("  %s test <message>      - Send message and test with different speeds\n", prog);
}

int main(int argc, char *argv[])
{
    int fd;
    int value;
    int original_duration;
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    /* Open device */
    fd = open("/dev/morse", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open /dev/morse");
        return 1;
    }
    
    if (strcmp(argv[1], "get_duration") == 0) {
        /* Get current unit duration */
        if (ioctl(fd, MORSE_GET_UNIT_DURATION, &value) == 0) {
            printf("Current unit duration: %d ms\n", value);
        } else {
            perror("GET_UNIT_DURATION failed");
        }
    }
    else if (strcmp(argv[1], "set_duration") == 0) {
        if (argc < 3) {
            printf("Usage: %s set_duration <ms>\n", argv[0]);
            close(fd);
            return 1;
        }
        value = atoi(argv[2]);
        if (ioctl(fd, MORSE_SET_UNIT_DURATION, &value) == 0) {
            printf("Set unit duration to: %d ms\n", value);
        } else {
            perror("SET_UNIT_DURATION failed");
        }
    }
    else if (strcmp(argv[1], "get_size") == 0) {
        /* Get current buffer size */
        if (ioctl(fd, MORSE_GET_BUFFER_SIZE, &value) == 0) {
            printf("Current buffer size: %d bytes\n", value);
        } else {
            perror("GET_BUFFER_SIZE failed");
        }
    }
    else if (strcmp(argv[1], "set_size") == 0) {
        if (argc < 3) {
            printf("Usage: %s set_size <bytes>\n", argv[0]);
            close(fd);
            return 1;
        }
        value = atoi(argv[2]);
        if (ioctl(fd, MORSE_SET_BUFFER_SIZE, &value) == 0) {
            printf("Set buffer size to: %d bytes\n", value);
        } else {
            perror("SET_BUFFER_SIZE failed");
        }
    }
    else if (strcmp(argv[1], "test") == 0) {
        if (argc < 3) {
            printf("Usage: %s test <message>\n", argv[0]);
            close(fd);
            return 1;
        }
        
        /* Get original duration */
        if (ioctl(fd, MORSE_GET_UNIT_DURATION, &original_duration) != 0) {
            perror("GET_UNIT_DURATION failed");
            close(fd);
            return 1;
        }
        printf("Original unit duration: %d ms\n", original_duration);
        
        /* Test with fast speed (50ms) */
        value = 50;
        printf("\n--- Testing FAST speed (50ms) ---\n");
        ioctl(fd, MORSE_SET_UNIT_DURATION, &value);
        write(fd, argv[2], strlen(argv[2]));
        sleep(3);  /* Wait for transmission */
        
        /* Test with normal speed (100ms) */
        value = 100;
        printf("\n--- Testing NORMAL speed (100ms) ---\n");
        ioctl(fd, MORSE_SET_UNIT_DURATION, &value);
        write(fd, argv[2], strlen(argv[2]));
        sleep(3);
        
        /* Test with slow speed (300ms) */
        value = 300;
        printf("\n--- Testing SLOW speed (300ms) ---\n");
        ioctl(fd, MORSE_SET_UNIT_DURATION, &value);
        write(fd, argv[2], strlen(argv[2]));
        sleep(5);
        
        /* Restore original duration */
        ioctl(fd, MORSE_SET_UNIT_DURATION, &original_duration);
        printf("\nRestored unit duration to: %d ms\n", original_duration);
    }
    else {
        printf("Unknown command: %s\n", argv[1]);
        print_usage(argv[0]);
    }
    
    close(fd);
    return 0;
}
