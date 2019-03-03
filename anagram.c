#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>

// lowers entire string
char *strlwr(char *str) {
  unsigned char *p = (unsigned char *)str;

  while (*p) {
    *p = tolower((unsigned char)*p);
    p++;
  }

  return str;
}

// debug print a char array (string)
void debug_string(unsigned char *str) {
  printf("\n%d[%c]", str[0], str[0]);
  for (int i = 1; i < strlen(str); i++) {
    printf(" - %i[%c]", str[i], str[i]);
  }
  printf("\n");
  return;
}

// Lowercase a single char if it is uppercase
char lowerIfHigher(unsigned char in) {
  char out;

  if (in > 40 && in < 91) {
    // ASCII capital
    out = in + 32;
    // printf ("%d - %c => %d - %c\n", in, in, out, out);
  } else {
    out = in;
  }

  return out;
}

// output array
int position = 0;
char results[4 * 1024] = {0};

// get line/word end, where the control charachters begin
const char *get_line_end(const char *p, const char *end) {
  while (p < end) {
    if (*p == '\n' || *p == '\r') {
      return p;
    }
    p++;
  }
  return end;
}

// get line/word beginning, where the control charachters end
const char *get_line_start(const char *p, const char *end) {
  while (p < end) {
    if (*p != '\n' && *p != '\r') {
      return p;
    }
    p++;
  }
  return end;
}

long getMicrotime() {
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

// define line_t
typedef union {
  char chars[32];
  __int128_t blocks[2];
} line_t;

// init with zeroes
line_t line = {0};

// charachteristics of input
int in_sum = 0;
int in_len = 0;
char in_max = 0;
char in_min = 255;

int main(int argc, char *argv[]) {
  long start = getMicrotime();
  nice(-20);

  // Beging handling of words with space
  if (argc == 3) {
    in_len = strlen(argv[2]);
  } else if (argc == 4) {
    in_len = strlen(argv[2]) + 1 + strlen(argv[3]);
  } else {
    exit(EXIT_FAILURE);
  }

  unsigned char *in;
  in = malloc(in_len);

  if (argc == 3) {
    memcpy(in, argv[2], in_len);
  } else if (argc == 4) {
    memcpy(in, argv[2], in_len);
    strcat(in, " ");
    strcat(in, argv[3]);
  } else {
    exit(EXIT_FAILURE);
  }

  // input string
  for (int i = 0; i < in_len; i++) {
    char c = in[i];
    in_sum += in[i];
    line.chars[i] = in[i];
    if (c > in_max) {
      in_max = in[i];
    }
    if (c < in_min) {
      in_min = in[i];
    }
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    printf("Broken \n");
    return EXIT_FAILURE;
  }

  struct stat stat;
  fstat(fd, &stat);

  const char *file_start = mmap(0, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  const char *file_end = file_start + stat.st_size;

  const char *line_start = get_line_start(file_start, file_end);
  const char *line_end = get_line_end(line_start, file_end);
  while (line_start < file_end) {
    // file is aplhabetic, if line starts with charachter that is higher than
    // the highest charachter in the input, then all following lines will also
    // be too high which means we can stop
    if (line_start[0] > in_max) {
      break;
    }

    // difference between start and end pointer locations is the length
    if ((line_end - line_start) == in_len) {
      int line_sum = 0;
      const char *i = line_start;
      while (i < line_end) {
        line_sum += *i;
        if (*i > in_max) {
          line_sum = 0;
          break;
        }
        if (*i < in_min) {
          line_sum = 0;
          break;
        }
        i++;
      }

      if (line_sum == in_sum) {
        line_t l = line;

        const char *i = line_start;
        while (i < line_end) {
          int been_replaced = 0;

          int b = 0;
          while (b < in_len) {
            if (l.chars[b] == *i) {
              l.chars[b] = 0;
              been_replaced = 1;
              break;
            }
            b++;
          }
          if (!been_replaced) {
            break;
          }
          i++;
        }

        if (l.blocks[1] == 0 && l.blocks[0] == 0) {
          results[position++] = ',';

          const char *c = line_start;
          while (c < line_end) {
            results[position++] = *c;
            c++;
          }
        }
      }
    }
    line_start = get_line_start(line_end, file_end);
    line_end = get_line_end(line_start, file_end);
  }

  long end = getMicrotime();
  printf("%d%s\n", (end - start), results);
  return 0;
}
