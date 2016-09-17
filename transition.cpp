#include <utility>

#include "connection.hpp"
#include "transition.hpp"

using namespace std;

namespace sc {

    /**
     * class TransitionInstance
     */

    TransitionInstance::TransitionInstance() = default;
    TransitionInstance::~TransitionInstance() = default;

    /**
     * class Transition
     */

    Transition::Transition( string&& id )
            : Component( move( id ) )
    {
    }

} // namespace sc