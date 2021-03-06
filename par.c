#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <mpi.h>

#define T 8
#define NUM_CHARS 256
#define LINE_LEN 1001
#define TEST_CASES 11000

typedef struct element {
  int code;
  long long count;
} element_t;

/*
 * O(n) sort
 */
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

/*
 * We could parallelize here, but it need a nested parallelism, which didn't performed well in our tests using a small number of processors
 */
void count_characters(char *line, element_t *occurrences_map) {
  long long local_count[NUM_CHARS], i;
  size_t len = strlen(line);

  for (i = 0; i < NUM_CHARS; i++) {
    local_count[i] = 0;
  }

  for (i = 0; i < len; i++) {
    int char_code = (unsigned char) line[i];
    local_count[char_code] += 1;
  }

  for (int i = 0; i < NUM_CHARS; i++) {
    if (local_count[i] > 0)  {
      element_t *new_element = malloc(sizeof(element_t));
      new_element->code = i;
      new_element->count = local_count[i];
      occurrences_map[i] = *new_element;
    }
  }
}

char *read_input(int *count) {
  char *input = malloc(sizeof(char) * TEST_CASES * LINE_LEN);

  char buffer[LINE_LEN];
  while (scanf(" %[^\n]", buffer) != EOF) {
    strcpy(&input[(*count) * LINE_LEN], buffer);
    *count = *count + 1;
  }
  input[(*count + 1) * LINE_LEN] = '\0';
  return input;
}

int main() {
  MPI_Init(NULL, NULL);
  char *input;
  element_t *occurrences_map;
  int rank, comm_size, count = 0, chunk_size;
  double wtime;
  // create datatype for string array
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

  // use rank 0 as master and calculate input
  if (rank == 0) {
    input = read_input(&count);
    // allocate results array;
   occurrences_map = malloc(sizeof(element_t) * count * NUM_CHARS);
    // calculate input in chunks so we can send to each process
    chunk_size = floor(count / comm_size);
  }
  wtime = omp_get_wtime();
  /* broadcast calculated chunk size*/
  MPI_Bcast(&chunk_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // allocate in each process a subset of input array
  char *input_subset = malloc(sizeof(char) * chunk_size * LINE_LEN);

  // Scatter input to all process
  MPI_Scatter(input, chunk_size * LINE_LEN, MPI_CHAR, input_subset,
              chunk_size * LINE_LEN, MPI_CHAR, 0, MPI_COMM_WORLD);

  // allocate results for each node
  element_t *occurrences_map_subset = malloc(sizeof(element_t) * chunk_size * NUM_CHARS);

  // parallelize for each line
//  #pragma omp parallel for num_threads(T) shared(occurrences_map, input_subset)
  for (int i = 0; i < chunk_size; i++) {
    count_characters(&input_subset[i * LINE_LEN], &occurrences_map_subset[i * NUM_CHARS]);
    counting_sort(&occurrences_map_subset[i * NUM_CHARS], 0, NUM_CHARS);
  }

  // send data back to root
  MPI_Gather(occurrences_map_subset,
             sizeof(element_t) * chunk_size * NUM_CHARS,
             MPI_BYTE,
             occurrences_map,
             sizeof(element_t) * chunk_size * NUM_CHARS,
             MPI_BYTE,
             0,
             MPI_COMM_WORLD);
  int printed = 0;
  // print result in root
  if (rank == 0) {
    for (int i = 0; i < count; i++) {
      for (int j = 0; j < NUM_CHARS; j++) {
        if (occurrences_map[(i * NUM_CHARS) + j].count > 0) {
          printf("%d - %lld \n",occurrences_map[(i * NUM_CHARS) + j].code, occurrences_map[(i * NUM_CHARS) + j].count);
          printed++;
        }
      }
      // use this to adjust not equal divisions (when count / comm_size is not an integer)
      if (printed != 0) printf("\n");
      printed = 0;
    }
  }

  MPI_Finalize();

  return 0;
}