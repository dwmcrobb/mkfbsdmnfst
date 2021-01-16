//===========================================================================
// @(#) $DwmPath: dwm/mkfbsdmnfst/tags/mkfbsdmnfst-1.0.0/DwmArgumentIO.hh 11856 $
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
//!  \file DwmArgumentIO.hh
//!  \brief iostream helpers for Argument values
//---------------------------------------------------------------------------

#ifndef _DWMARGUMENTIO_HH_
#define _DWMARGUMENTIO_HH_

#include <array>
#include <iostream>
#include <set>
#include <vector>

#include "DwmTupleHelpers.hh"

namespace Dwm {

  namespace ArgumentIO {

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <class T>
    std::ostream & operator << (std::ostream & os, const std::vector<T> & t)
    {
      std::string  sep;
      for (const auto & e : t) {
        os << sep << e;
        sep = ",";
      }
      return os;
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <class... Ts>
    std::ostream & operator << (std::ostream & os, const std::tuple<Ts...> & t)
    {
      namespace Tuples = Dwm::TupleHelpers;
      std::string   sep;
      Tuples::ForEach(t,
                      [&os, &sep] (auto && e)
                      { os << sep << e; sep = ","; });
      return os;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <class T>
    std::ostream & operator << (std::ostream & os, const std::set<T> & s)
    {
      std::string  sep;
      for (const auto & e : s) {
        os << sep << e;
        sep = ",";
      }
      return os;
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <class T1, class T2>
    std::ostream & operator << (std::ostream & os, const std::pair<T1,T2> & t)
    {
      os << t.first << ',' << t.second;
      return os;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <class T, size_t N>
    std::ostream & operator << (std::ostream & os, const std::array<T,N> & t)
    {
      std::string  sep;
      for (const auto & e : t) {
        os << sep << e;
        sep = ",";
      }
      return os;
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <class T>
    std::istream & operator >> (std::istream & is, std::vector<T> & v)
    {
      v.clear();
      while (is) {
        std::string  s;
        if (std::getline(is, s, ',')) {
          std::istringstream  iss(s);
          T  e;
          iss >> e;
          v.push_back(e);
        }
      }
      if (is.rdstate() & std::ios_base::eofbit) {
        is.clear();
      }
      return is;
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <class... Ts>
    std::istream & operator >> (std::istream & is, std::tuple<Ts...> & t)
    {
      namespace Tuples = Dwm::TupleHelpers;
      
      std::string  s;
      std::istringstream  iss;

      Tuples::ForEach(t,
                      [&is,&s] (auto && e)
                      {
                        std::getline(is, s, ',');
                        if constexpr (std::is_same<std::remove_reference_t<decltype(e)>, std::string>::value) {
                          e = s;
                        }
                        else {
                          std::istringstream  iss(s);
                          iss >> e;
                        }
                      });
      if (is.rdstate() & std::ios_base::eofbit) {
        is.clear();
      }
      return is;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    std::istream & operator >> (std::istream & is, std::string & s)
    {
      std::getline(is, s); // , ',');
      return is;
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <class T>
    std::istream & operator >> (std::istream & is, std::set<T> & t)
    {
      t.clear();
      while (is) {
        std::string  s;
        if (std::getline(is, s, ',')) {
          std::istringstream  iss(s);
          T  e;
          iss >> e;
          if (! t.insert(e).second) {
            is.setstate(std::ios_base::failbit);
            break;
          }
        }
      }
      if (is.rdstate() & std::ios_base::eofbit) {
        is.clear();
      }
      return is;
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <class T1, class T2>
    std::istream & operator >> (std::istream & is, std::pair<T1,T2> & t)
    {
      std::string  s;
      if (getline(is, s, ',')) {
        if constexpr (std::is_same<T1,std::string>::value) {
          t.first = s;
        }
        else {
          std::istringstream  iss(s);
          iss >> t.first;
        }
        getline(is, s);
        if constexpr (std::is_same<T2,std::string>::value) {
          t.second = s;
        }
        else {
          std::istringstream  iss(s);
          iss >> t.second;
        }
      }
      return is;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <class T, size_t N>
    std::istream & operator >> (std::istream & is, std::array<T,N> & a)
    {
      for (auto i = 0; i < N; ++i) {
        if (is) {
          std::string  s;
          if (std::getline(is, s, ',')) {
            std::istringstream  iss(s);
            T  e;
            iss >> e;
            a[i] = e;
          }
        }
        else {
          break;
        }
      }
      if (is.rdstate() & std::ios_base::eofbit) {
        is.clear();
      }
      return is;
    }
    
  }  // namespace ArgumentIO

}  // namespace Dwm

#endif  // _DWMMCLOCARGUMENTIO_HH_
