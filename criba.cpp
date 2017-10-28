#include <iostream>
#include <vector>

int main () {

	long long int to = 1000000000;

	std::vector<bool> is_prime(to, true);
	std::vector<int> primes;
	primes.push_back(2);

	for (long long int i = 0; i < to; i+=2) is_prime[i] = false;
	for (long long int i = 3; i <= to; i+=2) {
		if (is_prime[i]) {
			primes.push_back(i);
			for (long long int j = i * i; j < to; j+= i*2) is_prime[j] = false;
		}
	}

	std::cout << primes.size() << "\n";

}