#include <stdexcept>
#include <utility>

#include "input.hpp"

using namespace std;

namespace sc {

    Input::Input( string id )
        : Component( "input", move( id ) )
    {
    }

    void Input::subscribeInputChange( Input::InputChangeEvent::Handler handler )
    {
        return inputChangeEvent_.subscribe( move( handler ) );
    }

    void Input::raiseInputChange( ChannelValue const& value ) const
    {
        inputChangeEvent_( value );
    }

} // namespace sc