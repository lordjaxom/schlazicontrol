#include <stdexcept>
#include <utility>

#include "input.hpp"

using namespace std;

namespace sc {

    Input::Input( string id )
        : Component( "input", move( id ) )
    {
    }

    void Input::subscribeInputChange( InputChangeEvent::slot_type const& handler )
    {
        inputChangeEvent_.connect( handler );
    }

    void Input::subscribeInputChangeEx( InputChangeEvent::extended_slot_type const& handler )
    {
        inputChangeEvent_.connect_extended( handler );
    }

    void Input::raiseInputChange( ChannelValue const& value ) const
    {
        inputChangeEvent_( value );
    }

} // namespace sc