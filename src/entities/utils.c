#include "utils.h"

#include <allegro5/allegro5.h>
#include <stdio.h>
#include <string.h>

void must_init(_Bool test, const char* description) {
  if (test) return;

  fprintf(stderr, "couldn't initialize %s\n", description);
  exit(1);
}

int NumToDigits(int n) {
  // evita overflow em -INT_MIN
  long long x = n;
  int digits = 0;
  if (x < 0) {
    x = -x;
    // conta dígitos do valor absoluto
    do {
      digits++;
      x /= 10;
    } while (x > 0);
    return digits + 1; // inclui '-'
  }

  do {
    digits++;
    x /= 10;
  } while (x > 0);

  return digits;
}

void ClearKeyboardKeys(unsigned char* keyboard_keys) {
  memset(keyboard_keys, 0, ALLEGRO_KEY_MAX * sizeof(unsigned char));
}

void ShuffleArray(int* array, int size) {
  if (size > 1) {
    for (int i = size - 1; i > 0; i--) {
      int j = rand() % (i + 1);
      int tmp = array[i];
      array[i] = array[j];
      array[j] = tmp;
    }
  }
}