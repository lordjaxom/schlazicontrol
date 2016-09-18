#include <iostream>

#include "trackable.hpp"

using namespace std;

namespace sc {

    namespace detail {

        /**
         * class TrackableTracker
         */

        void TrackableTracker::announce()
        {
            ++count_;
        }

        void TrackableTracker::forget()
        {
            --count_;
        }

        void TrackableTracker::increase( size_t allocated )
        {
            allocated_ += allocated;
        }

        void TrackableTracker::decrease( std::size_t allocated )
        {
            allocated_ -= allocated;
        }

        void TrackableTracker::statistics( std::ostream& os ) const
        {
            os << "\n\tChannelBuffer : count: " << count_ << ", allocated: " << allocated_;
        }

        /**
         * class TrackableBase
         */

        TrackableBase::TrackableBase( TrackableTracker& tracker )
                : tracker_( &tracker )
        {
            tracker_->announce();
        }

        TrackableBase::TrackableBase( TrackableBase const& other )
                : tracker_( other.tracker_ )
        {
            increase( other.allocated_ );
        }

        TrackableBase::TrackableBase( TrackableBase&& other )
        {
            swap( allocated_, other.allocated_ );
        }

        TrackableBase::~TrackableBase()
        {
            decrease( allocated_ );
            tracker_->forget();
        }

        TrackableBase& TrackableBase::operator=( TrackableBase const& other )
        {
            decrease( allocated_ );
            increase( other.allocated_ );
            return *this;
        }

        TrackableBase& TrackableBase::operator=( TrackableBase&& other )
        {
            swap( allocated_, other.allocated_ );
            return *this;
        }

        void TrackableBase::increase( std::size_t allocated )
        {
            allocated_ += allocated;
            tracker_->increase( allocated );
        }

        void TrackableBase::decrease( std::size_t allocated )
        {
            allocated_ -= allocated;
            tracker_->decrease( allocated );
        }

        void TrackableBase::statistics( std::ostream& os ) const
        {
            os << "allocated: " << allocated_;
        }

    } // namespace detail

} // namespace sc