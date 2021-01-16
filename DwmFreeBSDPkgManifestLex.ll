%{
//===========================================================================
// @(#) $DwmPath$
//===========================================================================
//  Copyright (c) Daniel W. McRobb 2016, 2020
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
//!  \file DwmFreeBSDPkgManifestLex.ll
//!  \author Daniel W. McRobb
//!  \brief Lexer for Dwm::FreeBSDPkg::Manifest
//---------------------------------------------------------------------------

extern "C" {
  #include <stdarg.h>
  #include <stdio.h>
  
  //--------------------------------------------------------------------------
  //!  
  //--------------------------------------------------------------------------
  void pkgmnfsterror(const char *arg, ...)
  {
    va_list  ap;
    va_start(ap, arg);
    vfprintf(stderr, arg, ap);
    fprintf(stderr, ": %s at line %d\n", yytext, yylineno);
    return;
  }
}
  
#include <map>
#include <regex>
#include <set>
#include <string>
#include <vector>

using namespace std;

#include "DwmFreeBSDPkgManifest.hh"
#include "DwmFreeBSDPkgManifestParse.hh"

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
bool IsScriptName(int scriptToken)
{
  bool  rc = false;
  static const std::set<int>  scriptNames = {
    INSTALL,
    POSTINSTALL,
    PREINSTALL,
    DEINSTALL,
    POSTDEINSTALL,
    PREDEINSTALL,
    UPGRADE,
    POSTUPGRADE,
    PREUPGRADE
  };
  return (scriptNames.find(scriptToken) != scriptNames.end());
}
      
//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int GetStringToken(const char *str)
{
  static const std::map<std::string,int>  keywords = {
    { "arch",           ARCH },
    { "categories",     CATEGORIES },
    { "comment",        COMMENT },
    { "deps",           DEPS },
    { "desc",           DESC },
    { "files",          FILES },
    { "gname",          GNAME },
    { "licenselogic",   LICENSELOGIC },
    { "licenses",       LICENSES },
    { "maintainer",     MAINTAINER },
    { "name",           NAME },
    { "origin",         ORIGIN },
    { "perm",           PERM },
    { "prefix",         PREFIX },
    { "scripts",        SCRIPTS },
    { "install",        INSTALL },
    { "post-install",   POSTINSTALL },
    { "pre-install",    PREINSTALL },
    { "deinstall",      DEINSTALL },
    { "pre-deinstall",  PREDEINSTALL },
    { "post-deinstall", POSTDEINSTALL },
    { "upgrade",        UPGRADE },
    { "pre-upgrade",    PREUPGRADE },
    { "post-upgrade",   POSTUPGRADE },
    { "uname",          UNAME },
    { "version",        VERSION },
    { "www",            WWW }
  };

  int  rc = STRING;
  auto  i = keywords.find(str);
  if (i != keywords.end()) {
    rc = i->second;
  }
  return rc;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
std::string GetScriptName(const std::string & scriptNameMatch)
{
  std::string  rc;
  std::regex   rgx("[ ]+([^ ]+)[:]");
  std::smatch  m;
  regex_search(scriptNameMatch, m, rgx);
  if (m.size() > 1) {
    rc = m[1];
  }
  std::cerr << "ScriptName: " << rc << '\n';
  return rc;
}
  
%}

%option noyywrap yylineno
%option outfile="DwmFreeBSDPkgManifestLex.cc"
%option prefix="pkgmnfst"

m_scripts      ^scripts\:[ \t]*[|][\-][ \t]*[\n]  
m_scriptName   ^[ \t]{2}(((post-|post-de|pre-|pre-de|de)*install)|((pre-|post-)*upgrade))\:[ \t]+\|\-\n

%x x_scripts
%x x_quotedString

%%

<INITIAL>\:                     { return ':'; }
<INITIAL>\,                     { return ','; }
<INITIAL>\{                     { return '{'; }
<INITIAL>\}                     { return '}'; }
<INITIAL>\[                     { return '['; }
<INITIAL>\]                     { return ']'; }
<INITIAL>\"                     { BEGIN(x_quotedString); return '"'; }
<x_quotedString>([^"]|[\\"]["])+  { int tok = GetStringToken(yytext);
                                  if ((tok == STRING) || IsScriptName(tok)) {
                                    pkgmnfstlval.stringVal = 
                                      new std::string(yytext);
                                  }
                                  return tok; }
<x_quotedString>\"              { BEGIN(INITIAL); return '"'; }
<INITIAL>[^:,{}\[\]" \t\n]+     { pkgmnfstlval.stringVal =
                                    new std::string(yytext);
                                  return GetStringToken(yytext); }
<INITIAL>^[ \t]*\#.*\n
[ \t\n]+

%%
