#include <stdexcept>
#include <utility>

#include "output.hpp"

using namespace std;

namespace sc {

    Output::Output( string id )
        : Component( "output", move( id ) )
    {
    }

} // namespace sc