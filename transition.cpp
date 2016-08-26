#include "connection.hpp"
#include "transition.hpp"

using namespace std;

namespace sc {

    TransitionInstance::TransitionInstance() = default;
    TransitionInstance::~TransitionInstance() = default;

    Transition::Transition( string const& id )
            : Component( "transition", id )
    {
    }

} // namespace sc