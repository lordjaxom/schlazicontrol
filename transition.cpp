#include <utility>

#include "connection.hpp"
#include "transition.hpp"

using namespace std;

namespace sc {

    /**
     * class TransitionStateBase
     */

    TransitionStateBase::TransitionStateBase() = default;
    TransitionStateBase::~TransitionStateBase() = default;

    /**
     * class Transition
     */

    Transition::Transition( string id )
            : Component( "transition", move( id ) )
    {
    }

} // namespace sc