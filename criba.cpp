#include <iostream>
#include <vector>
#include <chrono>
#define tiempo std::chrono::high_resolution_clock::now
using namespace std;
int main () {

	long long int to = 1000000;

	std::vector<bool> is_prime(to, true);
	std::vector<int> primes;
	primes.push_back(2);
    auto start = tiempo();

	for (long long int i = 0; i < to; i+=2) is_prime[i] = false;
	for (long long int i = 3; i <= to; i+=2) {
		if (is_prime[i]) {
			primes.push_back(i);
			for (long long int j = i * i; j < to; j+= i*2) is_prime[j] = false;
		}
	}
	auto end = tiempo();

	cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() << endl;

	std::cout << primes.size() << "\n";
	return 0;
}