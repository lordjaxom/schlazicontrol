#include <stdexcept>

#include "output.hpp"

using namespace std;

namespace sc {

    Output::Output( string const& id )
        : Component( "output", id )
    {
    }

    std::size_t Output::channels() const
    {
        throw invalid_argument( "output components don't send channels" );
    }

} // namespace sc