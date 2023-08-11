#pragma once
#include <iterator>

namespace ql
{

  struct input_iterator_tag {};
  struct output_iterator_tag {};
  struct forward_iterator_tag : input_iterator_tag {};
  struct bidirectional_iterator_tag : forward_iterator_tag {};
  struct random_access_iterator_tag : bidirectional_iterator_tag {};
  struct contiguous_iterator_tag : public random_access_iterator_tag {};

  template<typename T>
  using iterator_traits = std::iterator_traits<T>;

  // advance
  // next
  // prev
  // distance

  namespace detail
  {

    template<typename Input>
    typename iterator_traits<Input>::difference_type distance( Input first, Input last, input_iterator_tag )
    {
      typename iterator_traits<Input>::difference_type result = 0;
      while ( first != last )
        ( first++, result++ );

      return result;
    }

    template<typename Input>
    typename iterator_traits<Input>::difference_type distance( Input first, Input last, random_access_iterator_tag )
    {
      return last - first;
    }

  }

  template<typename Input>
  typename iterator_traits<Input>::difference_type distance( Input first, Input last )
  {
    return detail::distance( first, last, typename iterator_traits<Input>::iterator_category() );
  }

}
