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


my_context g_c;

int FindPrimes::operator()( lli n ) const
{
    int factor = 3;
	int raiz = sqrt(n);

    while ( n % factor && factor <= raiz ) factor += 2;
    if (n % factor != 0) g_c.m_primes.put(n, n);

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
        from = atoi(argv[1]);
		to = atoi(argv[2]);
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

