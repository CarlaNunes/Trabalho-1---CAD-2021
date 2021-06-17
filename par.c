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
  long long count;
} element_t;

int compare(const void *a, const void *b) {
  element_t *element_a = (element_t *) a;
  element_t *element_b = (element_t *) b;

  if (element_a->count == element_b->count) return element_b->code - element_a->code;
  return element_a->count - element_b->count;
}

void count_characters(const char *line, int line_number, long long **global_count, element_t *occurrences_map) {
  long long local_count[NUM_CHARS], i;
  size_t len = strlen(line);

//  #pragma omp parallel for num_threads(T)
//  for (int i = 0; i < NUM_CHARS; i++) {
//    element_t *new_element = malloc(sizeof(element_t));
//    new_element->code = i;
//    new_element->count = 0;
//    count_map[i] = *new_element;
//  }

  omp_lock_t count_lock[NUM_CHARS];

  #pragma omp parallel for private(i) num_threads(T) shared(global_count, local_count)
  for (i = 0; i < NUM_CHARS; i++) {
    omp_init_lock(&count_lock[i]);
    local_count[i] = global_count[line_number][i] = 0;
  }

  #pragma omp parallel num_threads(T) private(i) firstprivate(local_count) shared(global_count)
  {
    #pragma omp for
    for (i = 0; i < len; i++) {
      int char_code = (unsigned char) line[i];
      local_count[char_code] += 1;
    }

    for (i = 0; i < NUM_CHARS; i++) {
      omp_set_lock(&count_lock[i]);
      global_count[line_number][i] += local_count[i];
      omp_unset_lock(&count_lock[i]);
    }
    #pragma omp barrier
  }

  // joga pra uma estretura de dados que vai ordenar
  #pragma omp parallel for private(i) shared(occurrences_map)
  for (int i = 0; i < NUM_CHARS; i++) {
    if (global_count[line_number][i] > 0)  {
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
  // podemos paralizar
  #pragma omp parallel for num_threads(T) shared(global_count, occurrences_map)
  for (int i = 0; i < count; i++) {
    count_characters(input[i], i, global_count, occurrences_map[i]);
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
  printf("wtime: %lf", wtime);
  return 0;
}
