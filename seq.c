#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include<omp.h>

#define NUM_CHARS 256
#define LINE_LEN 1001
#define TEST_CASES 11000

typedef struct element {
  int code;
  int count;
} element_t;

void count_characters(const char *line, element_t *count_map) {
  size_t len = strlen(line);

  for (int i = 32; i <= 127; i++) {
    element_t *new_element = malloc(sizeof(element_t));
    new_element->code = i;
    new_element->count = 0;
    count_map[i] = *new_element;
  }

  for (int i = 0; i < len; i++) {
    int char_code = (unsigned char) line[i];
    count_map[char_code].count = count_map[char_code].count + 1;
  }
}


void counting_sort(element_t *array, int begin, int size) {
  element_t ordered[size];
  unsigned int counting[NUM_CHARS];

  memset(counting, 0, sizeof(counting));

  for (int i = begin; i < begin + size; i++) {
    counting[array[i].count] += 1;
  }

  for (int i = 1; i <= NUM_CHARS; i++) {
    counting[i] += counting[i - 1];
  }

  for (int i = begin; i < begin + size; i++) {
    ordered[counting[array[i].count] - 1] = array[i];
    counting[array[i].count]--;
  }

  for (int i = begin, j = 0; i < begin + size && j < size; i++, j++) {
    array[i] = ordered[j];
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

  for (int i = 0; i < count; i++) {
    count_characters(input[i], occurrences_map[i]);
    counting_sort(occurrences_map[i], 0, NUM_CHARS);
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
