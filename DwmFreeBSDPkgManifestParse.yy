%{
//===========================================================================
// @(#) $DwmPath: dwm/libDwm/trunk/src/DwmFreeBSDPkgManifestParse.yy 11635 $
// @(#) $Id: DwmFreeBSDPkgManifestParse.yy 11635 2021-01-03 08:56:36Z dwm $
//===========================================================================
//  Copyright (c) Daniel W. McRobb 2016
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
//!  \file DwmFreeBSDPkgManifestParse.yy
//!  \author Daniel W. McRobb
//!  \ Dwm::FreeBSDPkg::Manifest implementation and parser
//---------------------------------------------------------------------------

extern int pkgmnfstlex();

extern "C" {
  #include <stdio.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  
  extern void pkgmnfsterror(const char *arg, ...);
  extern FILE *pkgmnfstin;
}

#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "DwmFreeBSDPkgManifest.hh"

static Dwm::FreeBSDPkg::Manifest *g_manifest = 0;

using namespace std;

%}

%define api.prefix {pkgmnfst}

%union {
  const string                                        *stringVal;
  Dwm::FreeBSDPkg::Manifest::Dependency               *depVal;
  std::vector<Dwm::FreeBSDPkg::Manifest::Dependency>  *depVecVal;
  Dwm::FreeBSDPkg::Manifest::File                     *fileVal;
  std::vector<Dwm::FreeBSDPkg::Manifest::File>        *fileVecVal;
  std::map<std::string,std::string>                   *stringMapVal;
  std::pair<std::string,std::string>                  *stringPairVal;
  std::vector<std::string>                            *stringVecVal;
}

%token ARCH CATEGORIES COMMENT DESC DEPS FILES GNAME LICENSELOGIC LICENSES
%token MAINTAINER NAME ORIGIN PERM PIPEHYPHEN PREFIX SCRIPTS UNAME VERSION WWW
%token <stringVal> DEINSTALL INSTALL POSTDEINSTALL POSTINSTALL POSTUPGRADE
%token <stringVal> PREDEINSTALL PREINSTALL PREUPGRADE SCRIPTNAME SCRIPTLINE
%token <stringVal> STRING UPGRADE

%type <stringVal> Arch Comment Desc LicenseLogic Maintainer Name Origin Prefix
%type <stringVal> QuotedString ScriptName StringValue Version Www
%type <depVal> Dependency
%type <depVecVal> DependencyList Dependencies
%type <fileVal> File
%type <fileVecVal> FileList Files
%type <stringPairVal> FileAttribute FileGroup FileOwner FilePermissions Script
%type <stringMapVal> FileAttributes ScriptMap Scripts
%type <stringVecVal> Categories CategoryList Licenses LicenseList

%%

Contents: Content { }
| Contents Content { };

Content: Dependencies {
  g_manifest->Dependencies(*$1);
  delete $1;
}
| Files {
  g_manifest->Files(*$1);
  delete $1;
}
| Desc {
  g_manifest->Description(*$1);
  delete $1;
}
| Prefix {
  g_manifest->Prefix(*$1);
  delete $1;
}
| Arch {
  g_manifest->Arch(*$1);
  delete $1;
}
| Www {
  g_manifest->WWW(*$1);
  delete $1;
}
| Categories {
  g_manifest->Categories(*$1);
  delete $1;
}
| LicenseLogic {
  g_manifest->LicenseLogic(*$1);
  delete $1;
}
| Licenses {
  g_manifest->Licenses(*$1);
  delete $1;
}
| Maintainer {
  g_manifest->Maintainer(*$1);
  delete $1;
}
| Comment {
  g_manifest->Comment(*$1);
  delete $1;
}
| Origin {
  g_manifest->Origin(*$1);
  delete $1;
}
| Name {
  g_manifest->Name(*$1);
  delete $1;
}
| Version {
  g_manifest->Version(*$1);
  delete $1;
}
| Scripts {
  typedef const std::string & (Dwm::FreeBSDPkg::Manifest::*ScriptSetFn)(const std::string &);
  std::map<std::string,ScriptSetFn> scriptSetters = {
    { "install",        &Dwm::FreeBSDPkg::Manifest::Install },
    { "post-install",   &Dwm::FreeBSDPkg::Manifest::PostInstall },
    { "pre-install",    &Dwm::FreeBSDPkg::Manifest::PreInstall },
    { "deinstall",      &Dwm::FreeBSDPkg::Manifest::Deinstall },
    { "post-deinstall", &Dwm::FreeBSDPkg::Manifest::PostDeinstall },
    { "pre-deinstall",  &Dwm::FreeBSDPkg::Manifest::PreDeinstall },
    { "upgrade",        &Dwm::FreeBSDPkg::Manifest::Upgrade },
    { "post-upgrade",   &Dwm::FreeBSDPkg::Manifest::PostUpgrade },
    { "pre-upgrade",    &Dwm::FreeBSDPkg::Manifest::PreUpgrade }
  };
  for (auto s : *$1) {
    auto it = scriptSetters.find(s.first);
    if (it != scriptSetters.end()) {
      (g_manifest->*(it->second))(s.second);
    }
  }
  delete $1;
};

Dependencies: '"' DEPS '"' ':' '{' DependencyList '}' {
  $$ = $6;
}
| DEPS ':' '{' DependencyList '}' {
  $$ = $4;
};

DependencyList: Dependency {
  $$ = new std::vector<Dwm::FreeBSDPkg::Manifest::Dependency>();
  $$->push_back(*$1);
}
| DependencyList ',' Dependency {
  $$->push_back(*$3);
  delete $3;
};

Dependency: StringValue ':' '{' Origin ',' Version '}' {
  $$ = new Dwm::FreeBSDPkg::Manifest::Dependency(*$1,*$4,*$6);
  delete $1;
  delete $4;
  delete $6;
}
| StringValue ':' '{' Origin '}' {
  $$ = new Dwm::FreeBSDPkg::Manifest::Dependency(*$1,*$4);
  delete $1;
  delete $4;
};

Categories: CATEGORIES ':' '[' CategoryList ']' { $$ = $4; };

CategoryList: StringValue {
  $$ = new std::vector<std::string>();
  $$->push_back(*$1);
}
| CategoryList ',' StringValue {
  $$->push_back(*$3);
  delete $3;
};

LicenseLogic: LICENSELOGIC ':' StringValue
{
  $$ = $3;
};

Licenses: LICENSES ':' '[' LicenseList ']' { $$ = $4; };

LicenseList: StringValue {
  $$ = new std::vector<std::string>();
  $$->push_back(*$1);
  delete $1;
}
| LicenseList ',' StringValue {
  $$->push_back(*$3);
  delete $3;
};

Scripts: SCRIPTS ':' '{' ScriptMap '}' {
  $$ = $4;
};

ScriptMap: Script {
  $$ = new std::map<std::string,std::string>();
  $$->insert(*$1);
  delete $1;
}
| ScriptMap ',' Script {
  $$->insert(*$3);
  delete $3;
};

Script: ScriptName ':' StringValue {
  $$ = new std::pair<std::string,std::string>(*$1, *$3);
  delete $1;
  delete $3;
};

ScriptName: INSTALL { $$ = $1; }
| POSTINSTALL { $$ = $1; }
| PREINSTALL { $$ = $1; }
| DEINSTALL { $$ = $1; }
| POSTDEINSTALL { $$ = $1; }
| PREDEINSTALL { $$ = $1; }
| UPGRADE { $$ = $1; }
| POSTUPGRADE { $$ = $1; }
| PREUPGRADE { $$ = $1; }
;

Files: '"' FILES '"' ':' '{' FileList '}' {
  $$ = $6;
}
| FILES ':' '{' FileList '}' {
  $$ = $4;
}
| FILES ':' '{' '}' {
  $$ = new std::vector<Dwm::FreeBSDPkg::Manifest::File>();
};

FileList: File {
  $$ = new std::vector<Dwm::FreeBSDPkg::Manifest::File>();
  $$->push_back(*$1);
  delete $1;
}
| FileList ',' File {
  $$->push_back(*$3);
  delete $3;
};

File: StringValue ':' '{' FileAttributes '}' {
  $$ = new Dwm::FreeBSDPkg::Manifest::File();
  $$->Path(*$1);
  auto  it = $4->find("uname");
  if (it != $4->end()) {
    $$->User(it->second);
  }
  it = $4->find("gname");
  if (it != $4->end()) {
    $$->Group(it->second);
  }
  it = $4->find("perm");
  if (it != $4->end()) {
    $$->Mode(strtoul(it->second.c_str(), 0, 8));
  }
  delete $1;
  delete $4;
}
| StringValue ':' StringValue {
  $$ = new Dwm::FreeBSDPkg::Manifest::File(*$1,*$3);
  delete $1;
  delete $3;
}
| StringValue {
  $$ = new Dwm::FreeBSDPkg::Manifest::File();
  $$->Path(*$1);
  delete $1;
};

FileAttributes: FileAttribute {
  $$ = new std::map<std::string,std::string>();
  $$->insert(*$1);
  delete $1;
}
| FileAttributes ',' FileAttribute {
  $$->insert(*$3);
  delete $3;
};

FileAttribute: FilePermissions { $$ = $1; }
| FileGroup { $$ = $1; }
| FileOwner { $$ = $1; };

FilePermissions: PERM ':' StringValue {
  $$ = new std::pair<std::string,std::string>("perm", *$3);
  delete $3;
};

FileGroup: GNAME ':' StringValue {
  $$ = new std::pair<std::string,std::string>("gname", *$3);
  delete $3;
};

FileOwner: UNAME ':' StringValue {
  $$ = new std::pair<std::string,std::string>("uname", *$3);
  delete $3;
};

Desc: DescKey ':' StringValue { $$ = $3; };
DescKey: '"' DESC '"' | DESC;

Prefix: PrefixKey ':' StringValue { $$ = $3; };
PrefixKey: '"' PREFIX '"' | PREFIX;

Arch: ArchKey ':' StringValue { $$ = $3; };
ArchKey: '"' ARCH '"' | ARCH;

Www: WwwKey ':' StringValue { $$ = $3; };
WwwKey: '"' WWW '"' | WWW;

Maintainer: MaintainerKey ':' StringValue { $$ = $3; };
MaintainerKey: '"' MAINTAINER '"' | MAINTAINER;

Comment: CommentKey ':' StringValue { $$ = $3; };
CommentKey: '"' COMMENT '"' | COMMENT;

Origin: OriginKey ':' StringValue { $$ = $3; };
OriginKey: '"' ORIGIN '"' | ORIGIN;

Name: NameKey ':' StringValue { $$ = $3; }
NameKey: '"' NAME '"' | NAME;

Version: VersionKey ':' StringValue { $$ = $3; };
VersionKey: '"' VERSION '"' | VERSION;

StringValue: QuotedString { $$ = $1; }
| STRING { $$ = $1; };

QuotedString: '"' STRING '"' { $$ = $2; }
| '"' '"' {
  $$ = new std::string("");
};

%%

using namespace std;

namespace Dwm {

  namespace FreeBSDPkg {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    Manifest::Dependency::Dependency(const string & name,
                                     const string & origin,
                                     const string & version)
        : _name(name), _origin(origin), _version(version)
    {}
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Dependency::Name() const
    {
      return _name;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Dependency::Name(const string & name)
    {
      _name = name;
      return _name;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Dependency::Version() const
    {
      return _version;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Dependency::Version(const string & version)
    {
      _version = version;
      return _version;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Dependency::Origin() const
    {
      return _origin;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string &
    Manifest::Dependency::Origin(const string & origin)
    {
      _origin = origin;
      return _origin;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    static ostream & operator << (ostream & os, const vector<string> & vs)
    {
      if (os) {
        if (! vs.empty()) {
          auto it = vs.begin();
          os << "\"" << *it++ << "\"";
          for ( ; it != vs.end(); ++it) {
            os << ", \"" << *it << "\"";
          }
        }
      }
      return os;
    }
        
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    ostream & operator << (ostream & os, const Manifest::Dependency & dep)
    {
      if (os) {
        os << "\"" << dep._name << "\":{\"origin\":\"" << dep._origin
           << "\",\"version\":\"" << dep._version << "\"}";
      }
      return os;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    Manifest::File::File(const string & path, const string sha256,
                         const string & user, const string group,
                         mode_t mode)
        : _path(path), _sha256(sha256), _user(user), _group(group),
          _mode(mode)
    {}
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::File::Path() const
    {
      return _path;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::File::Path(const string & path)
    {
      _path = path;
      return _path;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::File::SHA256() const
    {
      return _sha256;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::File::SHA256(const string & sha256)
    {
      _sha256 = sha256;
      return _sha256;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::File::User() const
    {
      return _user;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::File::User(const string & user)
    {
      _user = user;
      return _user;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::File::Group() const
    {
      return _group;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::File::Group(const string & group)
    {
      _group = group;
      return _group;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    mode_t Manifest::File::Mode() const
    {
      return _mode;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    mode_t Manifest::File::Mode(mode_t mode)
    {
      _mode = mode;
      return _mode;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    ostream & operator << (ostream & os, const Manifest::File & file)
    {
      if (os) {
        os << '"' << file._path;
#if 0
        if (! file._sha256.empty()) {
          os << "\":\"" << file._sha256 << "\"";
        }
        else {
#endif
          bool  needComma = false, openBracePrinted = false;
          if (! file.User().empty()) {
            os << "\":{uname: " << file.User();
            openBracePrinted = true;
            needComma = true;
          }
          if (! file.Group().empty()) {
            if (! openBracePrinted) {
              os << "\":{"; openBracePrinted = true;
            }
            if (needComma) {
              os << ", ";
            }
            os << "gname: " << file.Group();
            needComma = true;
          }
          if (file.Mode()) {
            if (! openBracePrinted) {
              os << "\":{";
              openBracePrinted = true;
            }
            if (needComma) {
              os << ", ";
            }
            os << "perm: " << oct <<showbase << file.Mode() << dec;
          }
          if (openBracePrinted) {
            os << '}';
          }
          else {
            os << '"';
          }
#if 0
        }
#endif
      }
      return os;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Name() const
    {
      return _name;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Name(const string & name)
    {
      _name = name;
      return _name;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Version() const
    {
      return _version;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Version(const string & version)
    {
      _version = version;
      return _version;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Origin() const
    {
      return _origin;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Origin(const string & origin)
    {
      _origin = origin;
      return _origin;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Comment() const
    {
      return _comment;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Comment(const string & comment)
    {
      _comment = comment;
      return _comment;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Description() const
    {
      return _description;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Description(const string & description)
    {
      _description = description;
      return _description;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Arch() const
    {
      return _arch;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Arch(const string & arch)
    {
      _arch = arch;
      return _arch;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::WWW() const
    {
      return _www;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::WWW(const string & www)
    {
      _www = www;
      return _www;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Maintainer() const
    {
      return _maintainer;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Maintainer(const string & maintainer)
    {
      _maintainer = maintainer;
      return _maintainer;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Prefix() const
    {
      return _prefix;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Prefix(const string & prefix)
    {
      _prefix = prefix;
      return _prefix;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::LicenseLogic() const
    {
      return _licenseLogic;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::LicenseLogic(const string & licenseLogic)
    {
      _licenseLogic = licenseLogic;
      return _licenseLogic;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const vector<string> & Manifest::Categories() const
    {
      return _categories;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const vector<string> &
    Manifest::Categories(const vector<string> & categories)
    {
      _categories = categories;
      return _categories;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::vector<std::string> & Manifest::Licenses() const
    {
      return _licenses;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::vector<std::string> &
    Manifest::Licenses(const std::vector<std::string> & licenses)
    {
      _licenses = licenses;
      return _licenses;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    size_t Manifest::Flatsize() const
    {
      return _flatsize;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    size_t Manifest::Flatsize(size_t flatsize)
    {
      _flatsize = flatsize;
      return _flatsize;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const vector<Manifest::Dependency> & Manifest::Dependencies() const
    {
      return _dependencies;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    vector<Manifest::Dependency> & Manifest::Dependencies()
    {
      return _dependencies;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const vector<Manifest::Dependency> &
    Manifest::Dependencies(const vector<Manifest::Dependency> & dependencies)
    {
      _dependencies = dependencies;
      return _dependencies;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Conflict() const
    {
      return _conflict;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Conflict(const string conflict)
    {
      _conflict = conflict;
      return _conflict;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const vector<Manifest::Option> & Manifest::Options() const
    {
      return _options;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    vector<Manifest::Option> & Manifest::Options()
    {
      return _options;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const vector<Manifest::File> & Manifest::Files() const
    {
      return _files;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    vector<Manifest::File> & Manifest::Files()
    {
      return _files;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const vector<Manifest::File> &
    Manifest::Files(const vector<Manifest::File> & files)
    {
      _files = files;
      return _files;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string & Manifest::Install() const
    {
      return _install;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string &
    Manifest::Install(const std::string & install)
    {
      _install = install;
      return _install;
    }

    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string & Manifest::PostInstall() const
    {
      return _postInstall;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string & Manifest::PostInstall(const std::string & postInstall)
    {
      _postInstall = postInstall;
      return _postInstall;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string & Manifest::PreInstall() const
    {
      return _preInstall;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string & Manifest::PreInstall(const std::string & preInstall)
    {
      _preInstall = preInstall;
      return _preInstall;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::PreDeinstall() const
    {
      return _preDeinstall;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::PreDeinstall(const string & preDeinstall)
    {
      _preDeinstall = preDeinstall;
      return _preDeinstall;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::PostDeinstall() const
    {
      return _postDeinstall;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::PostDeinstall(const string & postDeinstall)
    {
      _postDeinstall = postDeinstall;
      return _postDeinstall;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Deinstall() const
    {
      return _deinstall;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Deinstall(const string & deinstall)
    {
      _deinstall = deinstall;
      return _deinstall;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::PreUpgrade() const
    {
      return _preUpgrade;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::PreUpgrade(const string & preUpgrade)
    {
      _preUpgrade = preUpgrade;
      return _preUpgrade;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::PostUpgrade() const
    {
      return _postUpgrade;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::PostUpgrade(const string & postUpgrade)
    {
      _postUpgrade = postUpgrade;
      return _postUpgrade;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Upgrade() const
    {
      return _upgrade;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Manifest::Upgrade(const string & upgrade)
    {
      _upgrade = upgrade;
      return _upgrade;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Manifest::Parse(const char *filename)
    {
      bool  rc = false;
      g_manifest = this;
      pkgmnfstin = fopen(filename, "r");
      if (pkgmnfstin) {
        pkgmnfstparse();
        rc = true;
        fclose(pkgmnfstin);
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    vector<Manifest::File>
    Manifest::MissingFiles(const string & dirName) const
    {
      vector<Manifest::File>  missingFiles;
      string                  path;
      struct stat             statbuf;
      for (auto fit : _files) {
        path = dirName + fit.Path();
        if (stat(path.c_str(), &statbuf) != 0) {
          missingFiles.push_back(fit);
        }
      }
      return missingFiles;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    ostream & operator << (ostream & os,
                           const vector<Manifest::Dependency> & deps)
    {
      if (os) {
        auto dit = deps.begin();
        os << "{\n  " << *dit;
        ++dit;
        for ( ; dit != deps.end(); ++dit) {
          os << ",\n  " << *dit;
        }
        os << "\n}\n";
      }
      return os;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    ostream & operator << (ostream & os,
                           const vector<Manifest::File> & files)
    {
      if (os) {
        auto fit = files.begin();
        os << "{\n  " << *fit;
        ++fit;
        for ( ; fit != files.end(); ++fit) {
          os << ",\n  " << *fit;
        }
        os << "\n}\n";
      }
      return os;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    static string EscapeNewlines(const string & s)
    {
      regex  rgx("\\n");
      return regex_replace(s, rgx, "\\n");
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    ostream & operator << (ostream & os, const Manifest & manifest)
    {
      typedef const string & (Manifest::*StringFieldGetFn)() const;
      static const vector<pair<string,StringFieldGetFn> >  fieldGetters = {
        { "name",         &Manifest::Name },
        { "version",      &Manifest::Version },
        { "origin",       &Manifest::Origin },
        { "prefix",       &Manifest::Prefix },
        { "www",          &Manifest::WWW },
        { "maintainer",   &Manifest::Maintainer },
        { "comment",      &Manifest::Comment },
        { "desc",         &Manifest::Description },
        { "licenselogic", &Manifest::LicenseLogic }
      };
      if (os) {
        for (auto fgit : fieldGetters) {
          if (! (manifest.*(fgit.second))().empty()) {
            os << fgit.first << ": "
               << "\"" << (manifest.*(fgit.second))() << "\"\n";
          }
        }
        if (! manifest._licenses.empty()) {
          os << "licenses: [" << manifest._licenses << "]\n";
        }
        if (! manifest._categories.empty()) {
          os << "categories: [" << manifest._categories << "]\n";
        }
        if (! manifest._dependencies.empty()) {
          os << "deps: " << manifest._dependencies;
        }
        if (! manifest._files.empty()) {
          os << "files: " << manifest._files;
        }
        typedef const string & (Manifest::*ScriptGetFn)() const;
        static const vector<pair<string,ScriptGetFn> >  scriptGetters = {
          { "post-install",    &Manifest::PostInstall },
          { "pre-install",     &Manifest::PreInstall },
          { "install",         &Manifest::PreInstall },
          { "pre-deinstall",   &Manifest::PreDeinstall },
          { "post-deinstall",  &Manifest::PostDeinstall },
          { "deinstall",       &Manifest::Deinstall },
          { "pre-upgrade",     &Manifest::PreUpgrade },
          { "post-upgrade",    &Manifest::PostUpgrade },
          { "upgrade",         &Manifest::Upgrade }
        };
        bool  scriptsPrinted = false;
        bool  needComma = false;
        for (auto sgit : scriptGetters) {
          if (! (manifest.*(sgit.second))().empty()) {
            if (! scriptsPrinted) {
              os << "scripts: {";
              scriptsPrinted = true;
            }
            if (needComma) {
              os << ',';
            }
            os << "\n  " << sgit.first << ": \""
               << EscapeNewlines((manifest.*(sgit.second))()) << "\"";
            needComma = true;
          }
        }
        if (scriptsPrinted) {
          os << "\n}\n";
        }
      }
      return os;
    }
    
    
  }  // namespace FreeBSDPkg

}  // namespace Dwm
