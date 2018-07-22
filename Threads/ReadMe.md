Array, size of -n numbers given by the user, filed with random doubles.
Calculations performed in single thread mode.
After that -t number threads are created, and the same calculations divided between them. On the console output, we can see a boost in the efficiency of that method. Depending on how many threads was created the result will change.
My processor has 4 threads, this is maxed amount of threads that can boost calculations.
## Running the tests
You may type such commands:
* make all
* ./threads -t [number of threads] -n [number of elements in the array]

Or call help message.
* ./threads -h