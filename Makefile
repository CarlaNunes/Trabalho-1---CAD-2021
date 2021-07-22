all:
	mpicc diga_freq_par.c -o diga_freq_par -fopenmp -lm
clean:
	rm -rf diga_freq_par