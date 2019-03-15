#ifndef SCHLAZICONTROL_TRANSITION_HPP
#define SCHLAZICONTROL_TRANSITION_HPP

#include <memory>
#include <string>

#include "forward.hpp"
#include "core/input.hpp"
#include "core/output.hpp"

namespace sc {

    /**
     * class TransitionInstance
     */

    class TransitionInstance
    {
    public:
        TransitionInstance();
        TransitionInstance( TransitionInstance const& ) = delete;
        virtual ~TransitionInstance();

        virtual Transition const& transition() const = 0;

        virtual bool acceptsChannels( std::size_t channels ) const = 0;
        virtual std::size_t emitsChannels( std::size_t channels ) const = 0;

        virtual void transform( Connection& connection, ChannelBuffer& values ) = 0;
    };

    /**
     * class TransitionInstanceImpl
     */

    template< typename Transition, typename State = std::nullptr_t >
    class TransitionInstanceImpl final
            : public TransitionInstance
    {
    public:
        TransitionInstanceImpl( Transition const& transition )
                : transition_( transition )
                , state_()
        {
        }

        virtual Transition const& transition() const override { return transition_; }

        virtual bool acceptsChannels( std::size_t channels ) const override { return transition_.acceptsChannels( channels ); }
        virtual std::size_t emitsChannels( std::size_t channels ) const override { return transition_.emitsChannels( channels ); }

        virtual void transform( Connection& connection, ChannelBuffer& values ) override
        {
            transform( connection, values, state_ );
        }

    private:
        void transform( Connection& connection, ChannelBuffer& values, std::nullptr_t )
        {
            transition_.transform( connection, values );
        }

        template< typename Other >
        void transform( Connection& connection, ChannelBuffer& values, Other& state )
        {
            transition_.transform( state, connection, values );
        }

        Transition const& transition_;
        State state_;
    };

    /**
     * class Transition
     */

    class Transition
            : public Component
    {
    public:
        explicit Transition( std::string&& id );

        virtual std::unique_ptr< TransitionInstance > instantiate() const = 0;

    protected:
        virtual void doStatistics( std::ostream& os ) const override {}
    };

    /**
     * class TransitionRegistry
     */

    template< typename Type >
    class TransitionRegistry
        : public ComponentRegistry< Type >
    {
    public:
        explicit TransitionRegistry( char const* name )
                : ComponentRegistry< Type >( "transition", name )
        {
        }
    };

} // namespace sc

#endif // SCHLAZICONTROL_TRANSITION_HPP
