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
#include <cnc/cnc.h>
#include <cnc/debug.h>
#include <algorithm>
#include <cmath>

struct my_context;

typedef long long int lli;

struct FindPrimes
{
    int operator()( lli n ) const;
};

struct my_context : public CnC::context< my_context >
{
    CnC::item_collection< int,int > m_primes;
    my_context() 
        : CnC::context< my_context >(),
          m_primes( *this )
    {
        //CnC::debug::collect_scheduler_statistics( *this );
        
    }
};

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

my_context g_c;

int FindPrimes::operator()( lli n ) const
{
    if (rabin(n)) g_c.m_primes.put(n, n);
    return CnC::CNC_Success;
}


int main(int argc, char* argv[])
{
    bool verbose = false;
    lli from = 0;
    lli to = 0;
    int number_of_primes = 0;

    if (argc == 3) 
    {
        from = atol(argv[1]);
        to = atol(argv[2]);
    }
    else
    {
        fprintf(stderr,"Usage: from to\n");
        return -1;
    }


    printf("Determining primes from %lld-%lld \n", from, to);
    
    if (from % 2 == 0) from++;
    if (to % 2 == 0) to--;

    tbb::tick_count t0 = tbb::tick_count::now();

    CnC::parallel_for( from, to + 1, 2, FindPrimes(), CnC::pfor_tuner< false >() );

    tbb::tick_count t1 = tbb::tick_count::now();

    number_of_primes = (int)g_c.m_primes.size() + 1;
    printf("Found %d primes in %g seconds\n", number_of_primes, (t1-t0).seconds());

    if (number_of_primes <= 30)
    {
        printf("%d\n", 2);
        for (CnC::item_collection<int,int>::const_iterator cii = g_c.m_primes.begin(); cii != g_c.m_primes.end(); cii++) 
        {
            printf("%d\n", cii->first); // kludge
        }
    }
}

