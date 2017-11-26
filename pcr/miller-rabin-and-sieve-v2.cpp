#include <pcr/pcr.h>

using namespace fxml;

#include <ostream>

using std::pair;
using std::make_pair;
using std::vector;

typedef long long int num_t;

typedef pair<num_t,num_t> numPair;
vector<int> small_primes;
long long int MAX_CRIBA = 500000;
struct range {
    num_t from;
    num_t to;
};

#define forr(i,a,b) for(int i=(a); i<(b); i++)
#define forn(i,n) forr(i,0,n)
typedef long long int ll;

ll range_length = 10000;

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

long long int firstMultiple(ll n, ll p){
    ll num = n - n % p;
	if (num < n || num == 0) num += p;
	if (num == p) num += p;
	return num;
}

inline vector<long long> _criba(pcr_read<numPair>& m) {
	numPair sarasa = m;
	range range; range.from = sarasa.first; range.to = sarasa.second;
	long long cuenta = 0;
    std::vector<bool> is_prime((range.to - range.from) / 2, true);
    
	for (auto cii = small_primes.begin(); cii != small_primes.end(); cii++) {
		int primo = *cii;
		num_t first_multiple = firstMultiple(range.from, primo);
		if (first_multiple % 2 == 0) first_multiple += primo;
        for (num_t i = first_multiple; i < range.to; i += primo * 2) {
            is_prime[(i - range.from - 1) / 2] = false;    
        }
	}

	vector < num_t > ve;
	for (int i = 0; i < is_prime.size(); i++) {
		num_t num = range.from + i * 2 + 1;
        if (is_prime[i] && num > 1) {
			ve.push_back(num);
		}
    }
    return ve;
}
inline void rangos(pcr_read<numPair>& N, pcr_write<numPair>& O) {
    numPair t = N;
    for (num_t i = t.first; i < t.second; i+= range_length) {
		O = make_pair(i, std::min(i + range_length, t.second));
	}
}

inline int _count(pcr_read<num_t>& b, int& r) {
    if (b.idx() == 1) r = 0;
    r += num_t(b);
    return false;
}


inline num_t _determine(pcr_read<vector<num_t>>& pr) {
	vector<num_t> ve = pr; 
	long long a = 0;
	for( auto num : ve) {
		if (num <= MAX_CRIBA * MAX_CRIBA) {
			a += 1;
		} else {
			a += rabin(num);
		}
	}
	return a;
}


typedef producer<decltype(&rangos), &rangos>                    Rangos;
typedef unordered_consumer<decltype(&_criba), &_criba>      	Criba;
typedef unordered_consumer<decltype(&_determine), &_determine>  DeterminarSiSonONo;
typedef reducer<decltype(&_count), &_count>                     Count;

typedef pcr<
    pcr_in<numPair>,
    pcr_produce<Rangos, 1>,
    pcr_consume<Criba, 1>,
    pcr_consume<DeterminarSiSonONo, 1>,
    pcr_reduce<Count, 1>
> countprimes;

int main(int argc, char **argv) {
    if (argc < 3) return 1;

    pcr_impl<countprimes> p;
    CnC::debug::collect_scheduler_statistics(p.context) ;

    // Will count odd primer lesser than n
    long long from = atol(argv[1]);
    long long to = atol(argv[2]);

    int raiz = std::min((num_t) int(floor(sqrt(to))), MAX_CRIBA); 
	vector<bool> is_prime(ceil(raiz / 2) + 1, true);

	for (num_t i = 3; i <= raiz; i += 2) {
		if (is_prime[(i - 1) / 2]) {
			small_primes.push_back(i);
			for (num_t j = i * i; j <= raiz; j+= i*2) is_prime[(j - 1) / 2] = false;
		}
	}

	if (from % 2 == 1) from -= 1;
	if (to % 2 == 1) to += 1;
	range_length *=2;

    p(make_pair(from, to));

    p.wait();
    auto it = p.val_begin();
    while (it != p.val_end()) {
        std::cout << "There are " << *it << " odd primes smaller than " << argv[2] << std::endl;
        ++it;
    }

    return 0;
}