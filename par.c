#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include<omp.h>

#define T 16
#define NUM_CHARS 256
#define LINE_LEN 1001
#define TEST_CASES 11000 // quantidade de testes que serão rodados

typedef struct element {
  int code;
  long long count;
} element_t;

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

void count_characters(const char *line, int line_number, long long **global_count, element_t *occurrences_map) {
  long long local_count[NUM_CHARS];
  int i;
  size_t len = strlen(line);

  for (i = 0; i < NUM_CHARS; i++) {
    local_count[i] = global_count[line_number][i] = 0;
  }

  for (i = 0; i < len; i++) {
    int char_code = (unsigned char) line[i];
    local_count[char_code] += 1;
  }

  for (i = 32; i <= 127; i++) {
    global_count[line_number][i] += local_count[i];
  }

  for (i = 32; i <= 127; i++) {
    if (global_count[line_number][i] > 0) {
      element_t *new_element = malloc(sizeof(element_t));
      new_element->code = i;
      new_element->count = global_count[line_number][i];
      occurrences_map[i] = *new_element;

    }
  }
}

int main() {
  char *buffer = malloc(sizeof(char) * LINE_LEN);
  char **input = malloc(sizeof(char *) * TEST_CASES);
  for (int i = 0; i < TEST_CASES; i++) input[i] = malloc(sizeof(char) * LINE_LEN);

  element_t **occurrences_map = malloc(sizeof(element_t *) * TEST_CASES);
  for (int i = 0; i < TEST_CASES; i++) occurrences_map[i] = malloc(sizeof(element_t) * LINE_LEN);

  long long **global_count = malloc(sizeof(long long *) * TEST_CASES);
  for (int i = 0; i < TEST_CASES; i++) global_count[i] = malloc(sizeof(long long) * LINE_LEN);

  int count = 0;
  double wtime;

  // read_input
  while (scanf(" %[^\n]", buffer) != EOF) {
    strcpy(input[count], buffer);
    count++;
  }

  wtime = omp_get_wtime();
  #pragma omp parallel for num_threads(T) shared(global_count, occurrences_map)
  for (int i = 0; i < count; i++) {
    count_characters(input[i], i, global_count, occurrences_map[i]);
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
  for (int i = 0; i < TEST_CASES; i++) {
    free(input[i]);
    free(occurrences_map[i]);
  }
  free(input);
  free(occurrences_map);
  printf("wtime: %lf", wtime);
  return 0;
}
