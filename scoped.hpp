#ifndef SCHLAZICONTROL_SCOPED_HPP
#define SCHLAZICONTROL_SCOPED_HPP

#include <functional>

namespace sc {

    class Scoped
    {
    public:
        using Handler = std::function< void () >;

        Scoped( Handler release = []{} );
        Scoped( Handler acquire, Handler release );
        Scoped( Scoped const& ) = delete;
        Scoped( Scoped&& other );
        ~Scoped();

    private:
        Handler release_;
    };

} // namespace sc

#endif // SCHLAZICONTROL_SCOPED_HPP
