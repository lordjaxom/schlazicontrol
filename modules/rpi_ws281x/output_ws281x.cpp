#include <cstddef>
#include <limits>
#include <string>
#include <utility>

#include "core/input.hpp"
#include "core/manager.hpp"
#include "core/output.hpp"
#include "core/properties.hpp"
#include "modules/rpi_ws281x/ws281x.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    /**
     * class Ws281xOutput
     */

    static PropertyKey const ws281xProperty( "ws281x" );
    static PropertyKey const inputProperty( "input" );

    class Ws281xOutput final
            : public Output
    {
    public:
        Ws281xOutput( string&& id, Manager& manager, PropertyNode const& properties )
                : Component( move( id ) )
                , Output( manager, properties[ inputProperty ] )
                , ws281x_( manager.get< Ws281x >( *this, properties[ ws281xProperty ] ) ) {}

        bool acceptsChannels( size_t channels ) const override { return ws281x_.ledCount() * 3 == channels; }

    protected:
        void set( Input const& input, ChannelBuffer const& values ) override
        {
            ws281x_.send( values_ = values );
        }

        void doStatistics( std::ostream& os ) const override
        {
            os << "\n\t\tvalues: " << makeStatistics( values_ );
        }

    private:
        Ws281x& ws281x_;
        ChannelBuffer values_;
    };


    /**
     * registry
     */

    static OutputRegistry< Ws281xOutput > registry( "ws281x" );

} // namespace sc
