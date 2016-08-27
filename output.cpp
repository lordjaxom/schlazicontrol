#include <stdexcept>
#include <utility>

#include "output.hpp"

using namespace std;

namespace sc {

    Output::Output( string id )
        : Component( "output", move( id ) )
    {
    }

    std::size_t Output::channels() const
    {
        throw invalid_argument( "output components don't send channels" );
    }

} // namespace sc