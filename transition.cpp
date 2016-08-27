#include <utility>

#include "connection.hpp"
#include "transition.hpp"

using namespace std;

namespace sc {

    TransitionInstance::TransitionInstance() = default;
    TransitionInstance::~TransitionInstance() = default;

    Transition::Transition( string id )
            : Component( "transition", move( id ) )
    {
    }

} // namespace sc