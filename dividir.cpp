#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#define tiempo std::chrono::high_resolution_clock::now
using namespace std;
int main () {
	long long int from = 1;
	long long int to = 100000000;

	auto start = tiempo();
	int lala = 0;
	for(int i = 3; i < to; i++) {
		for (int j = 2; j <= floor(sqrt(i)); j++) {
			if (i % j == 0) {
				lala++;
				break;
			}
		}
	}
	auto end = tiempo();

	cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() / 1000000000.<< endl;
	
	std::cout << to - from - lala - 1 << "\n";
	return 0;
}