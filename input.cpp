#include <stdexcept>
#include <utility>

#include "input.hpp"

using namespace std;

namespace sc {

    Input::Input( string const& id )
        : Component( "input", id )
    {
    }

    EventConnection Input::subscribeInputChange( Input::InputChangeEvent::slot_function_type&& handler )
    {
        return inputChangeEvent_.connect( std::move( handler ) );
    }

    void Input::raiseInputChange( ChannelValue const& value ) const
    {
        inputChangeEvent_( value );
    }

    bool Input::acceptsChannels( std::size_t channels ) const
    {
        throw invalid_argument( "input components don't accept channels" );
    }

} // namespace sc