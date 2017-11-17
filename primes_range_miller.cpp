//********************************************************************************
// Copyright (c) 2007-2014 Intel Corporation. All rights reserved.              **
//                                                                              **
// Redistribution and use in source and binary forms, with or without           **
// modification, are permitted provided that the following conditions are met:  **
//   * Redistributions of source code must retain the above copyright notice,   **
//     this list of conditions and the following disclaimer.                    **
//   * Redistributions in binary form must reproduce the above copyright        **
//     notice, this list of conditions and the following disclaimer in the      **
//     documentation and/or other materials provided with the distribution.     **
//   * Neither the name of Intel Corporation nor the names of its contributors  **
//     may be used to endorse or promote products derived from this software    **
//     without specific prior written permission.                               **
//                                                                              **
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"  **
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE    **
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   **
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE     **
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR          **
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF         **
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS     **
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN      **
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)      **
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF       **
// THE POSSIBILITY OF SUCH DAMAGE.                                              **
//********************************************************************************
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tbb/tick_count.h>
#ifdef _DIST_
#include <cnc/dist_cnc.h>
#include <cnc/internal/dist/distributor.h>
#else
#include <cnc/cnc.h>
#endif

#include <math.h>
#include <vector>
#include <algorithm>
#include <cnc/debug.h>

struct my_context;

typedef long long int lli;

long long int MAX_CRIBA = 500000;

#define forr(i,a,b) for(int i=(a); i<(b); i++)
#define forn(i,n) forr(i,0,n)

typedef long long int ll;

ll mulmod (ll a, ll b, ll c) { //returns (a*b)%c, and minimize overfloor
    ll x = 0, y = a%c;
    while (b > 0){
        if (b % 2 == 1) x = (x+y) % c;
        y = (y*2) % c;
        b /= 2;
    }
    return x % c;
}

ll expmod (ll b, ll e, ll m){//O(log b)
    if(!e) return 1;
    ll q= expmod(b,e/2,m); q=mulmod(q,q,m);
    return e%2? mulmod(b,q,m) : q;
}

bool es_primo_prob (ll n, int a)
{
    if (n == a) return true;
    ll s = 0,d = n-1;
    while (d % 2 == 0) s++,d/=2;
    
    ll x = expmod(a,d,n);
    if ((x == 1) || (x+1 == n)) return true;
    
    forn (i, s-1){
        x = mulmod(x, x, n);
        if (x == 1) return false;
        if (x+1 == n) return true;
    }
    return false;
}
        
bool rabin (ll n){ //devuelve true si n es primo
    if (n == 1) return false;
    const int ar[] = {2,3,5,7,11,13,17,19,23};
    forn (j,9)
        if (!es_primo_prob(n,ar[j]))
            return false;
    return true;
}

struct range {
    lli from;
    lli to;
};

struct FindPrimes
{
    int execute(range range, my_context & c ) const;
};

struct RabinMiller {
	int execute(lli prime, my_context & c ) const; 
};

struct my_context : public CnC::context< my_context >
{
    CnC::step_collection< FindPrimes > find_primes;
	CnC::step_collection< RabinMiller > rabin_miller;
    CnC::tag_collection<range> m_tags;
	CnC::tag_collection<lli> possible_primes;
    CnC::item_collection< lli, lli > m_primes;

	CnC::item_collection< int, int > small_primes;

    my_context() 
        : CnC::context< my_context >(),
          find_primes( *this ),
          m_tags( *this ),
          m_primes( *this ),
		  small_primes(*this),
		  rabin_miller(*this),
		  possible_primes(*this)
    {
        m_tags.prescribes( find_primes, *this );
		possible_primes.prescribes(rabin_miller, *this);
		find_primes.consumes(small_primes);
		find_primes.produces(m_primes);
		rabin_miller.produces(m_primes);
		
    }
};


long long int firstMultple(lli n, lli p){
    lli num = n - n % p;
	if (num < n || num == 0) num += p;
	if (num == p) num += p;
	return num;
}

int RabinMiller::execute( lli possible_prime, my_context & c ) const
{
	if (rabin(possible_prime)) c.m_primes.put(possible_prime, possible_prime);
    return CnC::CNC_Success;
}

int FindPrimes::execute( range range, my_context & c ) const
{
    std::vector<bool> is_prime((range.to - range.from) / 2, true);
    
	for (auto cii = c.small_primes.begin(); cii != c.small_primes.end(); cii++) {
		int primo = cii -> first;
		lli first_multiple = firstMultple(range.from, primo);
		if (first_multiple % 2 == 0) first_multiple += primo;
        for (lli i = first_multiple; i < range.to; i += primo * 2) {
            is_prime[(i - range.from - 1) / 2] = false;    
        }
	}
    
    for (int i = 0; i < is_prime.size(); i++) {
		lli num = range.from + i * 2 + 1;
        if (is_prime[i] && num > 1) {
			if (num <= MAX_CRIBA * MAX_CRIBA) {
				c.m_primes.put(num, num);
			} else {
				c.possible_primes.put(num);
			}
		}
    }

    
    return CnC::CNC_Success;
}




int main(int argc, char* argv[])
{
#ifdef _DIST_
    CnC::dist_cnc_init< my_context > dc_init;
#endif
    lli from = 1;
	lli to = 0;
    int number_of_primes = 0;
	int range_length = 10000;

	if (argc == 4) {
		range_length = atol(argv[3]);
	}
    if (argc >= 3) 
    {
        from = _atoi64(argv[1]);
		to = _atoi64(argv[2]);
    }
    if (argc == 2)
    {
        to = atol(argv[1]);
    }
    if(argc <= 1 || argc > 4)
    {
        fprintf(stderr,"Usage: from to\n");
        return -1;
    }
	//CnC::debug::set_num_threads(3);
    my_context c;

    printf("Determining primes from %lld-%lld \n", from, to);

    tbb::tick_count t0 = tbb::tick_count::now();

	int raiz = std::min((lli) int(floor(sqrt(to))), MAX_CRIBA); 
	std::vector<bool> is_prime(ceil(raiz / 2) + 1, true);
	std::vector<int> small_primes;

	for (lli i = 3; i <= raiz; i += 2) {
		if (is_prime[(i - 1) / 2]) {
			small_primes.push_back(i);
			for (lli j = i * i; j <= raiz; j+= i*2) is_prime[(j - 1) / 2] = false;
		}
	}

	for (auto prime : small_primes) {
		c.small_primes.put(prime, prime);
	}

	if (from % 2 == 1) from -= 1;
	if (to % 2 == 1) to += 1;
	range_length *=2;

	for (lli i = from; i < to; i+= range_length) {
        range range;
        range.from = i;
        range.to = std::min(i + range_length, to);
		c.m_tags.put(range);
	}

    c.wait();

    tbb::tick_count t1 = tbb::tick_count::now();
	c.m_primes.put(2,2);
    // FIXME we have to transfer the items to the host first (distCnC)
    number_of_primes = (int)c.m_primes.size();
    printf("Found %d primes in %g seconds\n", number_of_primes, (t1-t0).seconds());

	if (number_of_primes < 30) {
		CnC::item_collection<lli,lli>::const_iterator cii;
		for (cii = c.m_primes.begin(); cii != c.m_primes.end(); cii++) 
		{
			printf("%d\n", cii->first); // kludge
		}
	}
    
}

