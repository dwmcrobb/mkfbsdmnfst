//===========================================================================
// @(#) $DwmPath: dwm/mkfbsdmnfst/tags/mkfbsdmnfst-1.0.0/DwmTupleHelpers.hh 11856 $
//===========================================================================
//  Copyright (c) Daniel W. McRobb 2020
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//  3. The names of the authors and copyright holders may not be used to
//     endorse or promote products derived from this software without
//     specific prior written permission.
//
//  IN NO EVENT SHALL DANIEL W. MCROBB BE LIABLE TO ANY PARTY FOR
//  DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
//  INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE,
//  EVEN IF DANIEL W. MCROBB HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.
//
//  THE SOFTWARE PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND
//  DANIEL W. MCROBB HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
//  UPDATES, ENHANCEMENTS, OR MODIFICATIONS. DANIEL W. MCROBB MAKES NO
//  REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER
//  IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE,
//  OR THAT THE USE OF THIS SOFTWARE WILL NOT INFRINGE ANY PATENT,
//  TRADEMARK OR OTHER RIGHTS.
//===========================================================================

//---------------------------------------------------------------------------
//!  \file DwmTupleHelpers.hh
//!  \brief tuple helper templates
//---------------------------------------------------------------------------

#ifndef _DWMTUPLEHELPERS_HH_
#define _DWMTUPLEHELPERS_HH_

#include <string>
#include <tuple>

namespace Dwm {

  namespace TupleHelpers {

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <class Tuple, class Func, std::size_t... I>
    constexpr Func ForEachImpl(Tuple && t, Func && f, std::index_sequence<I...>)
    {
      return (void)std::initializer_list<int>{(std::forward<Func>(f)(std::get<I>(std::forward<Tuple>(t))),0)...}, f;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <class Tuple, class Func>
    constexpr Func ForEach(Tuple && t, Func && f)
    {
      return ForEachImpl(std::forward<Tuple>(t), std::forward<Func>(f),
                         std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template<typename Tuple, typename Pred>
    constexpr size_t FindIf(Tuple && tuple, Pred pred)
    {
      size_t index = std::tuple_size<std::remove_reference_t<Tuple>>::value;
      size_t currentIndex = 0;
      bool found = false;
      ForEach(tuple, [&](auto&& value)
      {
        if (!found && pred(value)) {
          index = currentIndex;
          found = true;
        }
        ++currentIndex;
      });
      return index;
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template<typename Tuple, typename Action>
    void Perform(Tuple && tpl, size_t index, Action action)
    {
      size_t currentIndex = 0;
      ForEach(tpl,
              [action = std::move(action), index, &currentIndex]
              (auto && value)
              {
                if (currentIndex == index) {
                  action(std::forward<decltype(value)>(value));
                }
                ++currentIndex;
              });
    }

  }  // namespace TupleHelpers

}  // namespace Dwm

#endif  // _DWTUPLEHELPERS_HH_
