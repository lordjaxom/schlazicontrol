#ifndef SCHLAZICONTROL_TRACKABLE_HPP
#define SCHLAZICONTROL_TRACKABLE_HPP

#include <cstdint>
#include <iosfwd>

namespace sc {

    namespace detail {

        /**
         * class TrackableTracker
         */

        class TrackableTracker
        {
        public:
            void announce();
            void forget();

            void increase( std::size_t allocated );
            void decrease( std::size_t allocated );

            void statistics( std::ostream& os ) const;

        private:
            std::size_t count_ {};
            std::size_t allocated_ {};
        };

        /**
         * class TrackableBase
         */

        class TrackableBase
        {
        public:
            TrackableBase( TrackableTracker& tracker );
            TrackableBase( TrackableBase const& other );
            TrackableBase( TrackableBase&& other );
            ~TrackableBase();

            TrackableBase& operator=( TrackableBase const& other );
            TrackableBase& operator=( TrackableBase&& other );

            TrackableBase const& trackable() const { return *this; }
            TrackableTracker const& tracker() const { return *tracker_; }

            void statistics( std::ostream& os ) const;

        protected:
            template< typename Type >
            void allocate( std::size_t count )
            {
                increase( count * sizeof( Type ) );
            }

            template< typename Type >
            void deallocate( std::size_t count )
            {
                decrease( count * sizeof( Type ) );
            }

        private:
            void increase( std::size_t allocated );
            void decrease( std::size_t allocated );

            TrackableTracker* tracker_;
            std::size_t allocated_ {};
        };

    } // namespace detail

    /**
     * class Trackable
     */

    template< typename Derived >
    class Trackable : public detail::TrackableBase
    {
    public:
        using Trackable_ = Trackable;

        static detail::TrackableTracker const& tracker() { return tracker_; }

        Trackable()
                : detail::TrackableBase( tracker_ )
        {
        }

    private:
        static detail::TrackableTracker tracker_;
    };

    template< typename Derived >
    detail::TrackableTracker Trackable< Derived >::tracker_;

} // namespace sc

#endif // SCHLAZICONTROL_TRACKABLE_HPP
