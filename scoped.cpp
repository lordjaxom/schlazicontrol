#include "scoped.hpp"

using namespace std;

namespace sc {

    Scoped::Scoped( std::function< void () > handler )
            : handler_( move( handler ) )
    {
    }

    Scoped::Scoped( Scoped&& other )
            : handler_( move( other.handler_ ) )
    {
        other.handler_ = nullptr;
    }

    Scoped::~Scoped()
    {
        handler_();
    }

    void Scoped::push( std::function< void () > handler )
    {
        std::function< void () > previous = move( handler_ );
        handler_ = [previous, handler] { previous(); handler(); };
    }

} // namespace sc
