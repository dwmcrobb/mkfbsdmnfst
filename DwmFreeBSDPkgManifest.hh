//===========================================================================
// @(#) $Name:$
// @(#) $Id: DwmFreeBSDPkgManifest.hh 11856 2021-01-16 07:05:27Z dwm $
//===========================================================================
//  Copyright (c) Daniel W. McRobb 2016, 2017
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
//!  \file DwmFreeBSDPkgManifest.hh
//!  \brief class definitions for FreeBSD +MANIFEST file
//---------------------------------------------------------------------------

#ifndef _DWMFREEBSDPKGMANIFEST_HH_
#define _DWMFREEBSDPKGMANIFEST_HH_

#include <iostream>
#include <string>
#include <vector>

namespace Dwm {

  namespace FreeBSDPkg {

    //------------------------------------------------------------------------
    //!  Encapsulate a FreeBSD package manifest.  A manifest (+MANIFEST)
    //!  file is used when creating a FreeBSD package.  Some information on
    //!  the contents of a manifest file can be found in the pkg-create(8)
    //!  manpage.  This class is used by mkfbsdmnfst(1).
    //------------------------------------------------------------------------
    class Manifest
    {
    public:
      //----------------------------------------------------------------------
      //!  Encapsulate a package dependency, i.e. a package we depend on.
      //----------------------------------------------------------------------
      class Dependency
      {
      public:
        //--------------------------------------------------------------------
        //!  Default constructor
        //--------------------------------------------------------------------
        Dependency() = default;
        
        //--------------------------------------------------------------------
        //!  Construct from a package name, origin and version.
        //--------------------------------------------------------------------
        Dependency(const std::string & name, const std::string & origin,
                   const std::string & version = "");
        
        //--------------------------------------------------------------------
        //!  Return the dependency's name.
        //--------------------------------------------------------------------
        const std::string & Name() const;
        
        //--------------------------------------------------------------------
        //!  Set and return the dependency's name.
        //--------------------------------------------------------------------
        const std::string & Name(const std::string & name);
        
        //--------------------------------------------------------------------
        //!  Return the dependency's version.
        //--------------------------------------------------------------------
        const std::string & Version() const;
        
        //--------------------------------------------------------------------
        //!  Set and return the dependency's version.
        //--------------------------------------------------------------------
        const std::string & Version(const std::string & version);
        
        //--------------------------------------------------------------------
        //!  Return the dependency's origin.  This is a directory relative
        //!  to /usr/ports.
        //--------------------------------------------------------------------
        const std::string & Origin() const;
        
        //--------------------------------------------------------------------
        //!  Set and return the dependency's origin.  This is a directory
        //!  relative to /usr/ports.
        //--------------------------------------------------------------------
        const std::string & Origin(const std::string & origin);

        //--------------------------------------------------------------------
        //!  Print the dependency to an ostream.
        //--------------------------------------------------------------------
        friend std::ostream & operator << (std::ostream & os,
                                           const Dependency & dep);
      private:
        std::string  _name;
        std::string  _version;
        std::string  _origin;
      };

      //----------------------------------------------------------------------
      //!  Encapsulates a package manifest option.
      //----------------------------------------------------------------------
      class Option
      {
      public:
        const std::string & Name() const;
        const std::string & Name(const std::string & name);
        const std::string & Value() const;
        const std::string & Value(const std::string & value);

        friend std::ostream & operator << (std::ostream & os,
                                           const Option & option);

      private:
        std::string  _name;
        std::string  _value;
      };

      //----------------------------------------------------------------------
      //!  Encapsulate a file within a FreeBSD package manifest.
      //----------------------------------------------------------------------
      class File
      {
      public:
        File() = default;
        File(const std::string & path, const std::string sha256 = "",
             const std::string & user = "", const std::string group = "",
             mode_t mode = 0);
        const std::string & Path() const;
        const std::string & Path(const std::string & path);
        const std::string & SHA256() const;
        const std::string & SHA256(const std::string & sha256);
        const std::string & User() const;
        const std::string & User(const std::string & user);
        const std::string & Group() const;
        const std::string & Group(const std::string & group);
        mode_t Mode() const;
        mode_t Mode(mode_t mode);
        
        friend std::ostream & operator << (std::ostream & os,
                                           const File & file);
        
      private:
        std::string  _path;
        std::string  _sha256;
        std::string  _user;
        std::string  _group;
        mode_t       _mode;
      };
      
      //----------------------------------------------------------------------
      //!  Returns the package name from the manifest.
      //----------------------------------------------------------------------
      const std::string & Name() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the package name in the manifest.
      //----------------------------------------------------------------------
      const std::string & Name(const std::string & name);
      
      //----------------------------------------------------------------------
      //!  Returns the package version in the manifest.
      //----------------------------------------------------------------------
      const std::string & Version() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the package version in the manifest.
      //----------------------------------------------------------------------
      const std::string & Version(const std::string & version);
      
      //----------------------------------------------------------------------
      //!  Returns the package origin in the manifest.  This is a directory
      //!  relative to /usr/ports.
      //----------------------------------------------------------------------
      const std::string & Origin() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the package origin in the manifest.  This is a
      //!  directory relative to /usr/ports.
      //----------------------------------------------------------------------
      const std::string & Origin(const std::string & origin);
      
      //----------------------------------------------------------------------
      //!  Returns the package comment in the manifest.
      //----------------------------------------------------------------------
      const std::string & Comment() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the package comment in the manifest.
      //----------------------------------------------------------------------
      const std::string & Comment(const std::string & comment);
      
      //----------------------------------------------------------------------
      //!  Returns the package description in the manifest.
      //----------------------------------------------------------------------
      const std::string & Description() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the package description in the manifest.
      //----------------------------------------------------------------------
      const std::string & Description(const std::string & description);

      //----------------------------------------------------------------------
      //!  Return the package architecture in the manifest.
      //----------------------------------------------------------------------
      const std::string & Arch() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the package architecture in the manifest.
      //----------------------------------------------------------------------
      const std::string & Arch(const std::string & arch);
      
      //----------------------------------------------------------------------
      //!  Returns the package website (URL) in the manifest.
      //----------------------------------------------------------------------
      const std::string & WWW() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the package website (URL) in the manifest.
      //----------------------------------------------------------------------
      const std::string & WWW(const std::string & www);
      
      //----------------------------------------------------------------------
      //!  Returns the package maintainer in the manifest.
      //----------------------------------------------------------------------
      const std::string & Maintainer() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the package maintainer in the manifest.
      //----------------------------------------------------------------------
      const std::string & Maintainer(const std::string & maintainer);
      
      //----------------------------------------------------------------------
      //!  Returns the package prefix in the manifest.
      //----------------------------------------------------------------------
      const std::string & Prefix() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the package prefix in the manifest.
      //----------------------------------------------------------------------
      const std::string & Prefix(const std::string & prefix);
      
      //----------------------------------------------------------------------
      //!  Returns the package license logic in the manifest.
      //----------------------------------------------------------------------
      const std::string & LicenseLogic() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the package license logic in the manifest.
      //----------------------------------------------------------------------
      const std::string & LicenseLogic(const std::string & licenseLogic);
      
      //----------------------------------------------------------------------
      //!  Returns the package categories in the manifest.
      //----------------------------------------------------------------------
      const std::vector<std::string> & Categories() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the package categories in the manifest.
      //----------------------------------------------------------------------
      const std::vector<std::string> &
      Categories(const std::vector<std::string> & categories);
      
      //----------------------------------------------------------------------
      //!  Returns the package licenses in the manifest.
      //----------------------------------------------------------------------
      const std::vector<std::string> & Licenses() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the package licenses in the manifest.
      //----------------------------------------------------------------------
      const std::vector<std::string> &
      Licenses(const std::vector<std::string> & licenses);
      
      //----------------------------------------------------------------------
      //!  Returns the package's flatsize in the manifest.
      //----------------------------------------------------------------------
      size_t Flatsize() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the package's flatsize in the manifest.
      //----------------------------------------------------------------------
      size_t Flatsize(size_t flatsize);
      
      //----------------------------------------------------------------------
      //!  Returns a const reference to the package dependencies in the
      //!  manifest.
      //----------------------------------------------------------------------
      const std::vector<Dependency> & Dependencies() const;
      
      //----------------------------------------------------------------------
      //!  Returns a mutable reference to the package dependencies in the
      //!  manifest.
      //----------------------------------------------------------------------
      std::vector<Dependency> & Dependencies();
      
      //----------------------------------------------------------------------
      //!  Sets and returns the package dependencies in the manifest.
      //----------------------------------------------------------------------
      const std::vector<Dependency> &
      Dependencies(const std::vector<Dependency> & dependencies);

      //----------------------------------------------------------------------
      //!  Returns the package conflict in the manifest.
      //----------------------------------------------------------------------
      const std::string & Conflict() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the package conflict in the manifest.
      //----------------------------------------------------------------------
      const std::string & Conflict(const std::string conflict);
      
      //----------------------------------------------------------------------
      //!  Returns a const reference to the package options in the manifest.
      //----------------------------------------------------------------------
      const std::vector<Option> & Options() const;
      
      //----------------------------------------------------------------------
      //!  Returns a mutable reference to the package options in the
      //!  manifest.
      //----------------------------------------------------------------------
      std::vector<Option> & Options();

      //----------------------------------------------------------------------
      //!  Returns a const reference to the package files in the manifest.
      //----------------------------------------------------------------------
      const std::vector<File> & Files() const;
      
      //----------------------------------------------------------------------
      //!  Returns a mutable reference to the package files in the manifest.
      //----------------------------------------------------------------------
      std::vector<File> & Files();
      
      //----------------------------------------------------------------------
      //!  Sets and returns the package files in the manifest.
      //----------------------------------------------------------------------
      const std::vector<File> & Files(const std::vector<File> & files);

      //----------------------------------------------------------------------
      //!  Returns the package post-install in the manifest.
      //----------------------------------------------------------------------
      const std::string & PostInstall() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the package post-install in the manifest.
      //----------------------------------------------------------------------
      const std::string & PostInstall(const std::string & postInstall);

      //----------------------------------------------------------------------
      //!  Returns the package pre-install in the manifest.
      //----------------------------------------------------------------------
      const std::string & PreInstall() const;
 
      //----------------------------------------------------------------------
      //!  Sets and returns the package pre-install in the manifest.
      //----------------------------------------------------------------------
      const std::string & PreInstall(const std::string & preInstall);
      
      //----------------------------------------------------------------------
      //!  Returns the package install in the manifest.
      //----------------------------------------------------------------------
      const std::string & Install() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the package install in the manifest.
      //----------------------------------------------------------------------
      const std::string & Install(const std::string & install);

      //----------------------------------------------------------------------
      //!  Returns the package pre-deinstall in the manifest.
      //----------------------------------------------------------------------
      const std::string & PreDeinstall() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the package pre-deinstall in the manifest.
      //----------------------------------------------------------------------
      const std::string & PreDeinstall(const std::string & preDeinstall);

      //----------------------------------------------------------------------
      //!  Returns the package post-deinstall in the manifest.
      //----------------------------------------------------------------------
      const std::string & PostDeinstall() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the package post-deinstall in the manifest.
      //----------------------------------------------------------------------
      const std::string & PostDeinstall(const std::string & postDeinstall);

      //----------------------------------------------------------------------
      //!  Returns the package deinstall in the manifest.
      //----------------------------------------------------------------------
      const std::string & Deinstall() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the package deinstall in the manifest.
      //----------------------------------------------------------------------
      const std::string & Deinstall(const std::string & deinstall);

      //----------------------------------------------------------------------
      //!  Returns the package pre-upgrade in the manifest.
      //----------------------------------------------------------------------
      const std::string & PreUpgrade() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the package pre-upgrade in the manifest.
      //----------------------------------------------------------------------
      const std::string & PreUpgrade(const std::string & preUpgrade);

      //----------------------------------------------------------------------
      //!  Returns the package post-upgrade in the manifest.
      //----------------------------------------------------------------------
      const std::string & PostUpgrade() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the package post-upgrade in the manifest.
      //----------------------------------------------------------------------
      const std::string & PostUpgrade(const std::string & postUpgrade);

      //----------------------------------------------------------------------
      //!  Returns the package upgrade in the manifest.
      //----------------------------------------------------------------------
      const std::string & Upgrade() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the package upgrade in the manifest.
      //----------------------------------------------------------------------
      const std::string & Upgrade(const std::string & upgrade);
      
      //----------------------------------------------------------------------
      //!  Parses the manifest from the given @c filename.  Returns true
      //!  on success, false on failure.
      //----------------------------------------------------------------------
      bool Parse(const char *filename);

      //----------------------------------------------------------------------
      //!  Returns the files that were listed in the manifest but not found
      //!  in the given directory @c dirName.
      //----------------------------------------------------------------------
      std::vector<File> MissingFiles(const std::string & dirName) const;

      friend std::ostream & operator << (std::ostream & os,
                                         const Manifest & manifest);
      
    private:
      std::string               _name;
      std::string               _version;
      std::string               _origin;
      std::string               _comment;
      std::string               _description;
      std::string               _arch;
      std::string               _www;
      std::string               _maintainer;
      std::string               _prefix;
      std::string               _licenseLogic;
      std::vector<std::string>  _categories;
      std::vector<std::string>  _licenses;
      size_t                    _flatsize;
      std::vector<Dependency>   _dependencies;
      std::string               _conflict;
      std::vector<Option>       _options;
      std::vector<File>         _files;
      std::string               _postInstall;
      std::string               _preInstall;
      std::string               _install;
      std::string               _postDeinstall;
      std::string               _preDeinstall;
      std::string               _deinstall;
      std::string               _postUpgrade;
      std::string               _preUpgrade;
      std::string               _upgrade;
    };
    
    
  }  // namespace FreeBSDPkg

}  // namespace Dwm

#endif  // _DWMFREEBSDPKGMANIFEST_HH_
