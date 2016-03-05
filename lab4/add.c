

// straight from the spec
void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    *pointer = sum;
}