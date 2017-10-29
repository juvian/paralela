#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#define tiempo std::chrono::high_resolution_clock::now
using namespace std;
int main (int argc, char* argv[]) {
	long long int from = 1;
	long long int to = 1000;
    if (argc >= 3) 
    {
        from = atoi(argv[1]);
		to = atoi(argv[2]);
    }
	auto start = tiempo();
	if (from % 2 == 0) from++;
	if (to % 2 == 0) to--;

	int lala = 1;
	for(int n = 3; n <= to + 1; n+=2) {
		int factor = 3;
		int raiz = sqrt(n);
    	while ( n % factor && factor <= raiz ) factor += 2;
    	if (n % factor != 0) lala ++;
	}
	auto end = tiempo();

	cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() / 1000000000.<< endl;
	
	std::cout << lala << "\n";
	return 0;
}