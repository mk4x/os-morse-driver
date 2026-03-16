#ifndef MORSE_TABLE_H
#define MORSE_TABLE_H

#include <linux/types.h>

/*
 * Timing conventions (based on unit duration DU):
 *   dot        = 1 DU  on
 *   dash       = 3 DU  on
 *   symbol gap = 1 DU  off  (between dots/dashes within a letter)
 *   letter gap = 3 DU  off  (between letters)
 *   word gap   = 7 DU  off  (space character)
 */


/* Morse timing unit in milliseconds. */
extern int unit_duration;

/* 
Morse code encoding table for a-z, 0-9, and space.
Letters a-z, indexed as [0] = 'a', [1] = 'b', ... [25] = 'z' 
*/
extern const char *morse_letter[26];

/* Digits 0-9, indexed as [0] = '0', [1] = '1', ... [9] = '9' */
extern const char *morse_digit[10];

void transmit_morse(const char *buf, size_t len);
const char *morse_lookup(char c);


#endif /* MORSE_TABLE_H */