#include <pcr/pcr.h>

using namespace fxml;

#include <ostream>

using std::pair;
using std::make_pair;

typedef long long num_t;

typedef std::pair<num_t,num_t> numPair;


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

inline bool _isprime(pcr_read<num_t>& m) {
    num_t a = m;
    return rabin(a);
}

inline int _count(pcr_read<bool>& b, int& r) {
    if (b.idx() == 1) r = 0;
    r += bool(b);
    return false;
}

void odds(pcr_read<numPair> & N, pcr_write<num_t>& O) {
    numPair t = N;
    int first = t.first + (t.first % 2 == 0 ? 1 : 0 );
    while (first <= t.second) {
        O = first;
        first += 2;
    }
}

typedef producer<decltype(&odds), &odds>                        Odds;
typedef unordered_consumer<decltype(&_isprime), &_isprime>      isprime;
typedef reducer<decltype(&_count), &_count>                     Count;

typedef pcr<
    pcr_in<numPair>,
    pcr_produce<Odds, 1>,
    pcr_consume<isprime, 1>,
    pcr_reduce<Count,1>
> countprimes;

int main(int argc, char **argv) {
    if (argc < 3) return 1;

    pcr_impl<countprimes> p;
    CnC::debug::collect_scheduler_statistics(p.context) ;

    // Will count odd primer lesser than n
    long long left = atol(argv[1]);
    long long n = atol(argv[2]);
    p(make_pair(left,n));

    p.wait();
    auto it = p.val_begin();
    while (it != p.val_end()) {
        std::cout << "There are " << *it << " odd primes smaller than " << argv[2] << std::endl;
        ++it;
    }

    return 0;
}
