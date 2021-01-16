//===========================================================================
// @(#) $DwmPath: dwm/mkfbsdmnfst/tags/mkfbsdmnfst-1.0.0/DwmArguments.hh 11856 $
//===========================================================================
//  Copyright (c) Daniel W. McRobb 2019, 2020
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
//!  \file DwmArguments.hh
//!  \brief Command line argument processing class templates
//---------------------------------------------------------------------------

#ifndef _DWMARGUMENTS_HH_
#define _DWMARGUMENTS_HH_

extern "C" {
  #include <unistd.h>
}

#include <array>
#include <iomanip>
#include <regex>
#include <sstream>
#include <utility>

#include "DwmArgumentIO.hh"

namespace Dwm {

  //------------------------------------------------------------------------
  //!  Encapsulate a single argument.  C is the character of the argument,
  //!  ValueType is the type of the argument (bool for arguments that
  //!  that don't take a value), Req must be true for required arguments.
  //------------------------------------------------------------------------
  template <char C, typename ValueType, bool Req = false>
  class Argument
  {
  public:
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    Argument() : _value(), _valueName(), _help(), _conflicts()  { }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    Argument(const ValueType & v, const std::string & valueName = "",
             const std::string & help = "")
        : _value(v), _valueName(valueName), _help(help), _conflicts()
    { }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    static constexpr char OptChar()        { return _c; }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    const ValueType & Value() const        { return _value; }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    const ValueType & Value(const ValueType & v)
    {
      _value = v;
      return _value;
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    ValueType & Value()                    { return _value; }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    static constexpr bool Required()       { return _required; }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    static constexpr bool NeedsValue()     { return _needsValue; }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    const std::string & ValueName() const  { return _valueName; }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    const std::string & ValueName(const std::string & valueName)
    {
      _valueName = valueName;
      return _valueName;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    const std::string & Help() const       { return _help; }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    const std::string & Help(const std::string & help)
    {
      _help = help;
      return _help;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    const std::set<char> & Conflicts() const   { return _conflicts; }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    const std::set<char> & Conflicts(const std::set<char> & conflicts)
    {
      _conflicts = conflicts;
      return _conflicts;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    std::string UsageString() const
    {
      std::string  rc;
      if (isprint(_c)) {
        rc += '-';
        rc += _c;
      }
      if (_needsValue) {
        if (isprint(_c)) {
          rc += ' ';
        }
        rc += _valueName;
      }
      return rc;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    static std::string IndentedText(const std::string & s)
    {
      std::string  rc;
      std::string  ss;
      static const  std::set<char>  ws({' ','\t','\n'});
      size_t       col = 8;
      for (size_t i = 0; i < s.size(); ++i) {
        if (ws.find(s[i]) != ws.end()) {
          if ('\n' == s[i]) {
            rc += "\n\t";
            col = 8;
          }
          else {
            size_t  j = i+1;
            for ( ; (j < s.size()) && (ws.find(s[j]) == ws.end()); ++j) {
            }
            if ((col + (j - i)) > 76) {
              rc += "\n\t";
              col = 8;
            }
            else {
              rc += s[i];
              ++col;
            }
          }
        }
        else {
          rc += s[i];
          ++col;
        }
      }
      return rc;
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    std::string HelpText() const
    {
      std::string  rc(UsageString());
      if (NeedsValue()) {
        rc += '\n';
      }
      rc += '\t';
      rc += IndentedText(_help);
      return rc;
    }
      
  private:
    ValueType       _value;
    std::string     _valueName;
    std::string     _help;
    std::set<char>  _conflicts;
    
    static constexpr const char  _c = C;
    static constexpr const bool  _needsValue =
      (! std::is_same<bool, ValueType>::value);
    static constexpr const bool  _required = Req;
  };
  
  //------------------------------------------------------------------------
  //!  
  //------------------------------------------------------------------------
  template <class ...Ts>
  class Arguments
  {
  public:
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    Arguments(const char *envVar = nullptr)
        : _args()
    {
      LoadFromEnvironment(envVar);
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    Arguments(const char *envVar, Ts ...args)
        : _args({ args... })
    {
      LoadFromEnvironment(envVar);
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    int Parse(int argc, char *argv[])
    {
      bool         erc = false;
      std::string  optStr = OptString();
      int          optChar;

      _gotArgs.clear();
      while ((optChar = getopt(argc, argv, optStr.c_str())) != -1) {
        switch (optChar) {
          case '?':
            erc = true;
            break;
          default:
            if (ArgNeedsValue(optChar)) {
              if (Set(optChar, optarg)) { _gotArgs.insert(optChar); }
              else                      { erc = true;               }
            }
            else {
              if (Set(optChar, true))   { _gotArgs.insert(optChar); }
              else                      { erc = true;               }
            }
            KillConflicts(optChar);
            break;
        }
      }
      if ((! erc) && GotRequiredArgs() && (! GotConflictingArgs())) {
        return optind;
      }
      else {
        return -1;
      }
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    static constexpr auto Indexes()
    {
      return _indexes;
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    std::string OptString() const
    {
      std::string  rc;
      for (size_t i = 0; i < _indexes.size(); ++i) {
        if (isprint(_indexes.at(i).first)) {
          rc += _indexes.at(i).first;
          if (_indexes.at(i).second) {
            rc += ':';
          }
        }
      }
      return rc;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    std::set<std::string> UsageGroups() const
    {
      namespace Tuples = TupleHelpers;
      
      std::set<std::string>  rc;
      std::string            sep;
      std::set<std::pair<bool,std::set<std::string>>>  groups;
      Tuples::ForEach(_args,
                      [&sep,&groups,this] (auto e)
                      {
                        std::set<std::string>  ss;
                        ss.insert(e.UsageString());
                        for (auto conflict : e.Conflicts()) {
                          ss.insert(UsageString(conflict));
                        }
                        groups.insert(std::make_pair(e.Required(), ss));
                      });
      for (const auto & gr : groups) {
        std::string  grstr;
        sep.clear();
        if (! gr.first) {
          grstr += '[';
        }
        for (const auto & s : gr.second) {
          grstr += sep;
          grstr += s;
          sep = "|";
        }
        if (! gr.first) {
          grstr += ']';
        }
        rc.insert(grstr);
      }
      return rc;
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    std::string UsageString() const
    {
      namespace Tuples = TupleHelpers;
      
      std::string  rc;
      std::string  sep;

      std::set<std::pair<bool,std::set<std::string>>>  groups;
      Tuples::ForEach(_args,
                      [&sep,&rc,&groups,this] (auto e)
                      {
                        std::set<std::string>  ss;
                        ss.insert(e.UsageString());
                        for (auto conflict : e.Conflicts()) {
                          ss.insert(UsageString(conflict));
                        }
                        groups.insert(std::make_pair(e.Required(), ss));
                      });
      for (const auto & gr : groups) {
        sep.clear();
        if (! gr.first) {
          rc += '[';
        }
        for (const auto & s : gr.second) {
          rc += sep;
          rc += s;
          sep = "|";
        }
        if (! gr.first) {
          rc += ']';
        }
        rc += ' ';
      }
      return rc;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    std::string HelpText() const
    {
      namespace Tuples = TupleHelpers;
      
      std::string  rc;
      Tuples::ForEach(_args,
                      [&rc] (auto e)
                      {
                        rc += e.HelpText() + '\n';
                        if (! e.Conflicts().empty()) {
                          std::string  sep;
                          rc += "\t[conflicts with ";
                          for (auto conflict : e.Conflicts()) {
                            rc += sep;
                            rc += '-';
                            rc += conflict;
                            sep = ", ";
                          }
                          rc += "]\n";
                        }
                        rc += '\n';
                      });
      return rc;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    std::string Usage(const std::string & argv0,
                      const std::string & tailArgs = "") const
    {
      std::ostringstream  os;
      
      auto  groups = PrintableUsageGroups();
      size_t  indent = argv0.size() + std::string("Usage: ").size() + 1;
      
      os << "Usage: " << argv0 << ' ';
      int  col = indent;
      for (const auto & gr : groups) {
        if (col + gr.size() + 1 > 76) {
          os << '\n' << std::setw(indent) << ' ';
          col = indent;
        }
        os << gr << ' ';
        col += gr.size() + 1;
      }
      if (tailArgs.size() + col > 76) {
        os << '\n' << std::setw(indent) << ' ';
      }
      os << tailArgs << '\n' << '\n';
      os << HelpText();
      return os.str();
    }
      
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <char C>
    auto & Get() const
    {
      return std::get<Index(C)>(_args).Value();
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <char C>
    auto & Get()
    {
      return std::get<Index(C)>(_args).Value();
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <char C, typename V>
    void Set(const V & v)
    {
      std::get<Index(C)>(_args).Value(v);
      return;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <char C>
    void SetHelp(const std::string & help)
    {
      std::get<Index(C)>(_args).Help(help);
      return;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <char C>
    std::string GetHelp() const
    {
      return std::get<Index(C)>(_args).Help();
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <char C>
    std::string GetHelpText() const
    {
      return std::get<Index(C)>(_args).HelpText();
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <char C>
    void SetValueName(const std::string & valueName)
    {
      std::get<Index(C)>(_args).ValueName(valueName);
      return;
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    void SetConflicts(std::vector<std::set<char>> confs)
    {
      for (const auto & c : confs) {
        SetConflicts1(c);
      }
      return;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    bool LoadFromEnvironment(const char *envVar)
    {
      bool  rc = false;
      if (envVar) {
        using std::regex, std::sregex_iterator, std::regex_search,
          std::smatch, std::regex_match;
        const char *envVal = getenv(envVar);
        if (envVal) {
          rc = true;
          std::string  envStr(envVal);
          regex  rgx("[^:]+", regex::ECMAScript|regex::optimize);
          regex  rgx1("^[^=]", regex::ECMAScript|regex::optimize);
          regex  rgx2("^([^=])[=](.+)$", regex::ECMAScript|regex::optimize);
          auto  vbeg = sregex_iterator(envStr.begin(), envStr.end(), rgx);
          auto  vend = sregex_iterator();
          for (auto it = vbeg; it != vend; ++it) {
            smatch  match;
            std::string  itstr(it->str());
            if (regex_match(itstr, match, rgx2)) {
              if (match.size() == 3) {
                if (! Set(match[0].str()[0], match[2].str())) {
                  rc = false;
                  break;
                }
              }
            }
            else if (regex_match(itstr, match, rgx1)) {
              if (match.size() == 1) {
                if (! Set(match[0].str()[0], true)) {
                  rc = false;
                  break;
                }
              }
            }
            else {
              rc = false;
              break;
            }
          }
        }
      }
      return rc;
    }
        
  private:
    static constexpr const std::array<std::pair<char,bool>, std::tuple_size<std::tuple<Ts...>>::value>  _indexes = {std::pair<char,bool>(Ts::OptChar(),Ts::NeedsValue())...};
    
    std::tuple<Ts...>  _args;
    std::set<char>     _gotArgs;

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    static constexpr int Index(char c)
    {
      int  rc = -1;
      for (size_t i = 0; i < _indexes.size(); ++i) {
        if (_indexes.at(i).first == c) {
          rc = i;
          break;
        }
      }
      return rc;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    std::string UsageString(char c) const
    {
      namespace Tuples = TupleHelpers;
      
      std::string  usagestr;
      auto  idx = Tuples::FindIf(_args,
                                 [c](auto n)
                                 { return n.OptChar() == c; });
      Tuples::Perform(_args, idx,
                      [&usagestr] (auto n)
                      { usagestr = n.UsageString(); });
      return usagestr;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    void SetConflicts1(std::set<char> conflicts)
    {
      namespace Tuples = TupleHelpers;
      
      for (const auto conflict : conflicts) {
        auto  sc = conflicts;
        sc.erase(conflict);
        auto  idx = Tuples::FindIf(_args,
                                   [conflict] (auto n)
                                   { return n.OptChar() == conflict; });
        Tuples::Perform(_args, idx,
                        [&sc] (auto & n) { n.Conflicts(sc); });
      }
      return;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    constexpr bool ArgNeedsValue(char c) const
    {
      namespace Tuples = TupleHelpers;

      bool  rc = false;
      auto  idx = Tuples::FindIf(_args,
                                 [c] (auto n) { return n.OptChar() == c; });
      Tuples::Perform(_args, idx,
                      [&rc] (auto && n)
                      {
                        if (n.NeedsValue()) {
                          rc = true;
                        }
                      });
      return rc;
    }
    
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    template <typename U>
    bool Set(char c, const U u)
    {
      namespace Tuples = TupleHelpers;
      using namespace ArgumentIO;
      
      bool  rc = false;
      auto  idx = Tuples::FindIf(_args,
                                 [c] (auto n) { return n.OptChar() == c; });
      Tuples::Perform(_args, idx,
                      [&] (auto & n) {
                        std::ostringstream  os;
                        os << u;
                        std::istringstream  is(os.str());
                        if (is >> n.Value()) {
                          rc = true;
                        }
                      });
      return rc;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    bool Set(char c, std::string s)
    {
      namespace Tuples = TupleHelpers;
      using namespace ArgumentIO;
      
      std::istringstream  iss(s);
      bool  rc = false;
      auto  idx = Tuples::FindIf(_args,
                                 [c] (auto n) { return n.OptChar() == c; });
      Tuples::Perform(_args, idx,
                      [&] (auto & n) {
                        if (iss >> n.Value()) {
                          rc = true;
                        }
                      });
      return rc;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    void KillConflicts(char c)
    {
      namespace Tuples = TupleHelpers;
      std::set<char>  conflicts = GetConflicts(c);
      for (auto conflict : conflicts) {
        if (_gotArgs.find(conflict) == _gotArgs.end()) {
          auto  idx = Tuples::FindIf(_args,
                                     [conflict] (auto n)
                                     { return n.OptChar() == conflict; });
          Tuples::Perform(_args, idx,
                          [&] (auto & n)
                          {
                            if constexpr (std::is_same<std::remove_reference_t<decltype(n.Value())>,bool>::value) {
                              n.Value(false);
                            }
                            else {
                              n.Value(std::remove_reference_t<decltype(n.Value())>());
                            }
                          });
        }
      }
      return;
    }
        
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    std::set<char> GetConflicts(char c)
    {
      namespace Tuples = TupleHelpers;
      std::set<char>  rc;
      auto  idx = Tuples::FindIf(_args,
                                 [c] (auto n) { return n.OptChar() == c; });
      Tuples::Perform(_args, idx,
                      [&] (auto & n) {
                        rc = n.Conflicts();
                      });
      return rc;
    }
                      
    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    bool GotRequiredArgs()
    {
      namespace Tuples = TupleHelpers;
      bool  rc = true;
      Tuples::ForEach(_args,
                      [&rc, this] (auto e)
                      { if (e.Required()
                            && (_gotArgs.find(e.OptChar())
                                == _gotArgs.end())) {
                          rc = false;
                        }
                      });
      return rc;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    bool GotConflictingArgs()
    {
      namespace Tuples = TupleHelpers;
      bool  rc = false;
      Tuples::ForEach(_args,
                      [&rc, this] (auto e)
                      {
                        if (_gotArgs.find(e.OptChar())
                            != _gotArgs.end()) {
                          for (auto c : e.Conflicts()) {
                            if (_gotArgs.find(c)
                                != _gotArgs.end()) {
                              rc = true;
                              break;
                            }
                          }
                        }
                      });
      return rc;
    }

    //----------------------------------------------------------------------
    //!  
    //----------------------------------------------------------------------
    std::set<std::string> PrintableUsageGroups() const
    {
      namespace Tuples = TupleHelpers;
      std::set<std::string>  rc;
      std::string            sep;
      std::set<std::pair<bool,std::set<std::string>>>  groups;
      Tuples::ForEach(_args,
                      [&groups,this] (auto e)
                      {
                        if (isprint(e.OptChar())) {
                          std::set<std::string>  ss;
                          ss.insert(e.UsageString());
                          for (auto conflict : e.Conflicts()) {
                            ss.insert(this->UsageString(conflict));
                          }
                          groups.insert(std::make_pair(e.Required(), ss));
                        }
                      });
      for (const auto & gr : groups) {
        std::string  grstr;
        sep.clear();
        if (! gr.first) {
          grstr += '[';
        }
        for (const auto & s : gr.second) {
          grstr += sep;
          grstr += s;
          sep = "|";
        }
        if (! gr.first) {
          grstr += ']';
        }
        rc.insert(grstr);
      }
      return rc;
    }

  };

}  // namespace Dwm

#endif  // _DWMARGUMENTS_HH_
