all: pthread_benchmark green_benchmark

clean:
	rm -f pthread_benchmark green_benchmark small_test

pthread_benchmark:
	gcc -o pthread_benchmark pthread_benchmark.c -lpthread

green_benchmark:
	gcc -o green_benchmark green.c green_benchmark.c
