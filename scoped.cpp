#include <utility>

#include "scoped.hpp"

using namespace std;

namespace sc {

    Scoped::Scoped( Scoped::Handler release )
            : release_( move( release ) )
    {
    }

    Scoped::Scoped( Scoped::Handler acquire, Scoped::Handler release )
            : release_( move( release ) )
    {
        acquire();
    }

    Scoped::Scoped( Scoped&& other )
            : release_( move( other.release_ ) )
    {
        other.release_ = nullptr;
    }

    Scoped::~Scoped()
    {
        try {
            release_();
        }
        catch ( ... ) {}
    }

} // namespace sc
