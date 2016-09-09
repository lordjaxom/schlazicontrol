#include <stdexcept>
#include <utility>

#include "input.hpp"

using namespace std;

namespace sc {

    Input::Input( string id )
        : Component( "input", move( id ) )
    {
    }

} // namespace sc