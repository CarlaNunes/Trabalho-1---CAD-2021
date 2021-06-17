#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include<omp.h>

#define T 8
#define NUM_CHARS 256
#define LINE_LEN 1001
#define TEST_CASES 11000

typedef struct element {
  int code;
  int count;
} element_t;

int compare(const void *a, const void *b) {
  element_t *element_a = (element_t *) a;
  element_t *element_b = (element_t *) b;

  if (element_a->count == element_b->count) return element_b->code - element_a->code;
  return element_a->count - element_b->count;
}

void count_characters(const char *line, element_t *count_map) {
  size_t len = strlen(line);

  for (int i = 0; i < NUM_CHARS; i++) {
    element_t *new_element = malloc(sizeof(element_t));
    new_element->code = i;
    new_element->count = 0;
    count_map[i] = *new_element;
  }

  // podemos paralelizar como no exemplo dele sem reduction
  for (int i = 0; i < len; i++) {
    int char_code = (unsigned char) line[i];
    count_map[char_code].count = count_map[char_code].count + 1;
  }
}

int main() {
  char *buffer = malloc(sizeof(char) * LINE_LEN);
  char **input = malloc(sizeof(char *) * TEST_CASES);
  for (int i = 0; i < TEST_CASES; i++) input[i] = malloc(sizeof(char) * LINE_LEN);

  element_t **occurrences_map = malloc(sizeof(element_t *) * TEST_CASES);
  for (int i = 0; i < TEST_CASES; i++) occurrences_map[i] = malloc(sizeof(element_t) * LINE_LEN);

  int count = 0;
  double wtime;

  // read_input
  while (scanf(" %[^\n]", buffer) != EOF) {
    strcpy(input[count], buffer);
    count++;
  }

  wtime = omp_get_wtime();
  // podemos paralizar

  for (int i = 0; i < count; i++) {
    count_characters(input[i], occurrences_map[i]);
    qsort(occurrences_map[i], NUM_CHARS, sizeof(element_t), compare);
  }
  wtime = omp_get_wtime() - wtime;

  for (int i = 0; i < count; i++) {
    for (int j = 0; j < NUM_CHARS; j++) {
      if (occurrences_map[i][j].count > 0)
        printf("%d - %d \n", occurrences_map[i][j].code, occurrences_map[i][j].count);
    }
    printf("\n");
  }


  free(buffer);
  for(int i = 0; i < TEST_CASES; i++) {
    free(input[i]);
    free(occurrences_map[i]);
  }
  free(input);
  free(occurrences_map);
  printf("time taken: %lf", wtime);
  return 0;
}
