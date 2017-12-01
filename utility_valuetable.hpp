#ifndef SCHLAZICONTROL_UTILITY_VALUETABLE_HPP
#define SCHLAZICONTROL_UTILITY_VALUETABLE_HPP

#include <cstdlib>
#include <limits>

namespace sc {

	namespace detail {

		template< typename R, typename I >
		struct ValueTable_function_traits
		{
			typedef R result_type;
			typedef I input_type;
		};

		template< typename R, typename I >
		ValueTable_function_traits< R, I > get_ValueTable_function_traits( R ( *func )( I ) );

		template< typename F >
		struct ValueTable_traits
		{
			typedef decltype( get_ValueTable_function_traits( &F::apply ) ) function_traits;
			typedef typename function_traits::result_type result_type;
			typedef typename function_traits::input_type input_type;
		};

		template<
			typename F,
			typename ValueTable_traits< F >::input_type I,
			typename ValueTable_traits< F >::input_type To,
			typename ValueTable_traits< F >::result_type... Values
		>
		struct ValueTable_values
		{
			typedef ValueTable_traits< F > traits_type;
			typedef typename traits_type::result_type result_type;
			typedef typename traits_type::input_type input_type;

			typedef ValueTable_values< F, I + 1, To, Values..., F::apply( I ) > next_type;
			static decltype( next_type::values ) constexpr& values = next_type::values;
		};

		template<
			typename F,
			typename ValueTable_traits< F >::input_type I,
			typename ValueTable_traits< F >::input_type To,
			typename ValueTable_traits< F >::result_type... Values
		>
		decltype( ValueTable_values< F, I, To, Values... >::next_type::values ) constexpr& ValueTable_values< F, I, To, Values... >::values;

		template<
			typename F,
			typename ValueTable_traits< F >::input_type To,
			typename ValueTable_traits< F >::result_type... Values
		>
		struct ValueTable_values< F, To, To, Values... >
		{
			typedef ValueTable_traits< F > traits_type;
			typedef typename traits_type::result_type result_type;

			static result_type constexpr const values[] { Values..., F::apply( To ) };
		};


		template<
			typename F,
			typename ValueTable_traits< F >::input_type To,
			typename ValueTable_traits< F >::result_type... Values
		>
		typename ValueTable_traits< F >::result_type constexpr const ValueTable_values< F, To, To, Values... >::values[];

	} // namespace detail

	template<
		typename F,
		typename detail::ValueTable_traits< F >::input_type From = std::numeric_limits< typename detail::ValueTable_traits< F >::input_type >::min(),
		typename detail::ValueTable_traits< F >::input_type To   = std::numeric_limits< typename detail::ValueTable_traits< F >::input_type >::max()
	>
	class ValueTable
	{
		typedef detail::ValueTable_traits< F > traits_type;
		typedef typename traits_type::result_type result_type;
		typedef typename traits_type::input_type input_type;

		typedef detail::ValueTable_values< F, From, To > values_type;

	public:
		static result_type constexpr get( input_type input )
		{
			return values_type::values[ input - From ];
		}
	};

} // namespace sc

#endif // SCHLAZICONTROL_UTILITY_VALUETABLE_HPP
