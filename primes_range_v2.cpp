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

struct tag_type {
    lli from;
    lli to;
    std::vector<int> primes;
};

struct tag_type2 {
    lli from;
    lli to;
	int prime;
	std::vector<bool> &is_prime;

	tag_type2 (std::vector<bool> &is_prime2) : is_prime(is_prime2) {}
};

struct tag_type3 {
	lli to;
	lli from;
	std::vector<bool> &is_prime;
	std::vector<int> primes;

	tag_type3 (std::vector<bool> &is_prime2) : is_prime(is_prime2) {}
};

struct AddPrimes
{
    int execute(tag_type3 tag, my_context & c ) const;
};

struct FindPrimes
{
    int execute(tag_type range, my_context & c ) const;
};

struct CrossPrimes
{
    int execute(tag_type2 tag, my_context & c ) const;
};

struct my_context : public CnC::context< my_context >
{
    CnC::step_collection< FindPrimes > m_steps;
    CnC::tag_collection<tag_type> m_tags;
    CnC::step_collection< CrossPrimes > m_steps2;
    CnC::tag_collection<tag_type2> m_tags2;
    CnC::tag_collection<tag_type3> m_tags3;
    CnC::step_collection< AddPrimes > m_steps3;

    CnC::item_collection< lli, lli > m_primes;
	CnC::item_collection< std::pair<lli, int>, bool > m_done;

    my_context() 
        : CnC::context< my_context >(),
          m_steps( *this ),
		  m_steps2(*this),
		  m_steps3(*this),
          m_tags( *this ),
		  m_tags2(*this),
		  m_tags3(*this),
          m_primes( *this ),
          m_done( *this )
    {
        m_tags.prescribes( m_steps, *this );
		m_tags2.prescribes(m_steps2, *this);
		m_tags3.prescribes(m_steps3, *this);

    }

	//CnC::debug::collect_scheduler_statistics( ctxt );
};


long long int firstMultple(lli n, lli p){
    lli num = n - n % p;
	if (num < n || num == 0) num += p;
	if (num == p) num += p;
	return num;
}

int FindPrimes::execute(tag_type range, my_context & c) const
{
    std::vector<bool> is_prime((range.to - range.from) / 2, true);
        
    for (int primo : range.primes) {
        lli first_multiple = firstMultple(range.from, primo);
		if (first_multiple % 2 == 0) first_multiple += primo;
        tag_type2 tag(is_prime);
		tag.from = first_multiple;
		tag.to = range.to;
		tag.prime = primo;
		c.m_tags2.put(tag);
    }

    
	tag_type3 tag(is_prime);
	tag.to = range.to;
	tag.primes = range.primes;
	tag.from = range.from;
	c.m_tags3.put(tag);
    
    return CnC::CNC_Success;
}

int CrossPrimes::execute(tag_type2 tag, my_context & c) const {
	for (lli i = tag.from; i < tag.to; i += tag.prime * 2) {
        tag.is_prime[(i - tag.from - 1) / 2] = false;    
    }

	c.m_done.put(std::make_pair(tag.to, tag.prime), true);
	 
	return CnC::CNC_Success;
}

int AddPrimes::execute(tag_type3 tag,  my_context & c) const {
	for (auto prime : tag.primes) {
		bool val;
		printf("execute1");
		c.m_done.get(std::make_pair(tag.to, prime), val);
		printf("execute2");
	}

    for (int i = 0; i < tag.is_prime.size(); i++) {
		lli num = tag.from + i * 2 + 1;
        if (tag.is_prime[i] && num > 1) c.m_primes.put(num, num);
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
		range_length = atoi(argv[3]);
	}
    if (argc >= 3) 
    {
        from = atoi(argv[1]);
		to = atoi(argv[2]);
    }
    if (argc == 2)
    {
        to = atoi(argv[1]);
    }
    if(argc <= 1 || argc > 4)
    {
        fprintf(stderr,"Usage: from to\n");
        return -1;
    }
	//CnC::debug::set_num_threads(3);
    my_context c;

    printf("Determining primes from %d-%d \n", from, to);

    tbb::tick_count t0 = tbb::tick_count::now();

	int raiz = floor(sqrt(to)); 

	std::vector<bool> is_prime(ceil(raiz / 2) + 1, true);
	std::vector<int> primes;

	for (int i = 3; i <= raiz; i += 2) {
		if (is_prime[(i - 1) / 2]) {
			primes.push_back(i);
			for (int j = i * i; j <= raiz; j+= i*2) is_prime[(j - 1) / 2] = false;
		}
	}

	if (from % 2 == 1) from -= 1;
	if (to % 2 == 1) to += 1;
	range_length *=2;

	for (lli i = from; i < to; i+= range_length) {
        tag_type range;
        range.from = i;
        range.to = std::min(i + range_length, to);
        range.primes = primes;
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

