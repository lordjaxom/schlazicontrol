#include <chrono>
#include <fstream>
#include <iostream>
#include <regex>

#include <asio.hpp>

#include "core/input.hpp"
#include "core/logging.hpp"
#include "core/manager.hpp"
#include "core/properties.hpp"
#include "types.hpp"

using namespace std;

namespace sc {

    static Logger logger( "alsastatus" );

    namespace detail {

        struct AlsaStatusInfo final
        {
            string path;

            friend ostream& operator<<( ostream& os, AlsaStatusInfo const& info )
            {
                return os << "AlsaStatus{" << info.path << "}: ";
            }
        };

    } // namespace detail

    static PropertyKey const pathProperty( "path" );
    static PropertyKey const delayProperty( "delay" );

    class AlsaStatus final
            : public Input
    {
    public:
        AlsaStatus( string&& id, Manager& manager, PropertyNode const& properties )
                : Component( move( id ))
                , info_ { properties[ pathProperty ].as< string >() }
                , delay_( properties[ delayProperty ].as< chrono::nanoseconds >())
                , timer_( manager.service() )
        {
            manager.readyEvent().subscribe( [this] { wait(); } );
        }

        ~AlsaStatus() override = default;

        std::size_t emitsChannels() const override { return 1; }

    protected:
        void doStatistics( std::ostream& os ) const override
        {
        }

    private:
        void wait()
        {
            timer_.expires_after( delay_ );
            timer_.async_wait( [this]( asio::error_code ec ) {
                if ( ec ) {
                    return;
                }

                ifstream ifs( info_.path, ios::in );
                if ( !ifs ) {
                    logger.error( info_, "couldn't read status: ", strerror( errno ));
                }

                string line;
                getline( ifs, line );
                set( line != "closed" );

                wait();
            } );
        }

        void set( bool value )
        {
            if ( value_ != value ) {
                value_ = value;
                inputChangeEvent_( ChannelValue( value_ ));
            }
        }

        detail::AlsaStatusInfo info_;
        chrono::nanoseconds delay_;
        asio::steady_timer timer_;
        bool value_ {};
    };


    /**
     * registry
     */

    static InputRegistry< AlsaStatus > registry( "alsaStatus" );

} // namespace sc