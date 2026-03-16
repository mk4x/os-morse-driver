#include "morse_table.h"
#include "gpio_handler.h"
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/module.h>

int unit_duration = 100;

const char *morse_letter[26] = {
    ".-",    /* a */
    "-...",  /* b */
    "-.-.",  /* c */
    "-..",   /* d */
    ".",     /* e */
    "..-.",  /* f */
    "--.",   /* g */
    "....",  /* h */
    "..",    /* i */
    ".---",  /* j */
    "-.-",   /* k */
    ".-..",  /* l */
    "--",    /* m */
    "-.",    /* n */
    "---",   /* o */
    ".--.",  /* p */
    "--.-",  /* q */
    ".-.",   /* r */
    "...",   /* s */
    "-",     /* t */
    "..-",   /* u */
    "...-",  /* v */
    ".--",   /* w */
    "-..-",  /* x */
    "-.--",  /* y */
    "--..",  /* z */
};

const char *morse_digit[10] = {
    "-----", /* 0 */
    ".----", /* 1 */
    "..---", /* 2 */
    "...--", /* 3 */
    "....-", /* 4 */
    ".....", /* 5 */
    "-....", /* 6 */
    "--...", /* 7 */
    "---..", /* 8 */
    "----.", /* 9 */
};

/*
 * morse_lookup - get the Morse string for a character
 *
 * Returns the Morse string for letters (a-z, A-Z) and digits (0-9).
 * Returns NULL for unsupported characters.
 * Space is handled separately in the transmit logic (word gap).
 */

void transmit_morse(const char *buf, size_t len)
{
    size_t i;
 
    for (i = 0; i < len; i++) {
        char c = buf[i];
 
        if (c == ' ' || c == '\n') {
            /* word gap: 7 units — but 3 units of letter gap already */
            msleep(7 * unit_duration);
 
        } else {
            const char *code = morse_lookup(c);
            int j;
 
            if (!code) {
                /* unsupported character, skip */
                continue;
            }
 
            /* blink each dot or dash */
            for (j = 0; code[j] != '\0'; j++) {
                led_on();
                /* Now wait some time, dash or dot */
                if (code[j] == '.')
                    msleep(unit_duration);          /* dot  = 1 unit */
                else
                    msleep(3 * unit_duration);      /* dash = 3 units */
                led_off();
 
                /* inter-symbol gap (between dots/dashes in same letter) */
                if (code[j + 1] != '\0')
                    msleep(unit_duration);
            }
 
            /* inter-letter gap = 3 units total.
             * We already waited 1 unit after the last symbol,
             * so sleep 2 more to reach 3. */
            msleep(2 * unit_duration);
        }
    }
}


const char *morse_lookup(char c)
{
    if (c >= 'a' && c <= 'z')
        return morse_letter[c - 'a'];
    if (c >= 'A' && c <= 'Z')
        return morse_letter[c - 'A'];
    if (c >= '0' && c <= '9')
        return morse_digit[c - '0'];
    return NULL; /* unsupported — caller handles space and gaps */
}

/* Export symbols for other modules */
EXPORT_SYMBOL(transmit_morse);
EXPORT_SYMBOL(morse_lookup);

MODULE_AUTHOR( "...Mar-Joh, Jen-Wil" );
MODULE_LICENSE( "GPL" );