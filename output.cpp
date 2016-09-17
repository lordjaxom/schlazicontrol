#include <algorithm>
#include <utility>

#include "input.hpp"
#include "manager.hpp"
#include "output.hpp"

using namespace std;

namespace sc {

    void Output::checkConnection(
            Component const& input, Component const& output, size_t emitsChannels, bool acceptsChannels )
    {
        if ( !acceptsChannels ) {
            throw runtime_error( str(
                    "invalid connection between ", input.name(), " ", input.category(), " \"", input.id(), "\" and ",
                    output.name(), " ", output.category(), " \"", output.id(), "\": ", output.category(),
                    " doesn't accept ", emitsChannels, " output channels" ) );
        }
    }

    void Output::initialize( Manager& manager, PropertyNode const& inputsNode, false_type )
    {
        auto& input = manager.get< Input >( *this, inputsNode );
        manager.readyEvent().subscribe( [this, &input] { setup( input ); }, true );
        input_ = &input;
    }

    void Output::initialize( Manager& manager, PropertyNode const& inputsNode, true_type )
    {
        for_each(
                inputsNode.begin(), inputsNode.end(),
                [this, &manager]( PropertyNode const& node ) { initialize( manager, node ); } );
        input_ = nullptr; // not valid for multiconnections
    }

    void Output::setup( Input& input )
    {
        auto channels = input.emitsChannels();
        checkConnection( input, *this, channels, acceptsChannels( channels ) );
        input.inputChangeEvent().subscribe( [this, &input]( ChannelBuffer const& values ) { set( input, values ); } );
    }

} // namespace sc