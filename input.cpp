#include <utility>

#include "input.hpp"
#include "output.hpp"

using namespace std;

namespace sc {

    /**
     * free functions
     */

    void checkConnection( Component const& input, Component const& output, std::size_t channels, bool accepts )
    {
        if ( !accepts ) {
            throw runtime_error( str(
                    "invalid connection between ", input.name(), " ", input.category(), " \"", input.id(), "\" and ",
                    output.name(), " ", output.category(), " \"", output.id(), "\": ", output.category(),
                    " doesn't accept ", channels, " output channels" ) );
        }
    }

    void checkConnection( Input const& input, Output const& output )
    {
        size_t channels = input.emitsChannels();
        checkConnection( input, output, channels, output.acceptsChannels( channels ) );
    }

    void connect( Input& input, Output const& output, Input::InputChangeEvent::Handler handler )
    {
        checkConnection( input, output );
        input.inputChangeEvent().subscribe( move( handler ) );
    }

} // namespace sc