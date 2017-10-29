#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <stdlib.h>     /* atoi */

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
	for(int n = from; n <= to; n+=2) {
		int factor = 3;
		int raiz = sqrt(n);
    	while ( n % factor && factor <= raiz ) factor += 2;
    	if (n % factor != 0) lala ++;
	}
	auto end = tiempo();

	cout << std::chrono::duration<double>(end-start).count() << endl;
	
	std::cout << lala << "\n";
	return 0;
}