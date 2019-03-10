#include <algorithm>
#include <utility>

#include "core/input.hpp"
#include "manager.hpp"
#include "core/output.hpp"

using namespace std;

namespace sc {

    void Output::checkConnection(
            Component const& input, Component const& output, size_t emitsChannels, bool acceptsChannels )
    {
        if ( !acceptsChannels ) {
            throw runtime_error( str( "invalid connection between ", input.describe(), " and ", output.describe(),
                                      " receiver doesn't accept ", emitsChannels, " output channels" ) );
        }
    }

    void Output::initialize( Manager& manager, PropertyNode const& inputsNode, SingleInputTag )
    {
        auto& input = manager.get< Input >( *this, inputsNode );
        manager.readyEvent().subscribe( [this, &input] { setup( input ); }, true );
        inputs_.emplace_back( &input );
    }

    void Output::initialize( Manager& manager, PropertyNode const& inputsNode, MultipleInputsTag )
    {
        for_each(
                inputsNode.begin(), inputsNode.end(),
                [this, &manager]( PropertyNode const& node ) { initialize( manager, node ); } );
    }

    void Output::setup( Input& input )
    {
        auto channels = input.emitsChannels();
        checkConnection( input, *this, channels, acceptsChannels( channels ) );
        input.inputChangeEvent().subscribe( [this, &input]( ChannelBuffer const& values ) { set( input, values ); } );
    }

} // namespace sc