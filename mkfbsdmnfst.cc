//===========================================================================
// @(#) $DwmPath: dwm/mkfbsdmnfst/tags/mkfbsdmnfst-1.0.0/mkfbsdmnfst.cc 11856 $
// @(#) $Id: mkfbsdmnfst.cc 11856 2021-01-16 07:05:27Z dwm $
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
//!  \file mkfbsdmnfst.cc
//!  \brief mkfbsdmnfst application code including main()
//---------------------------------------------------------------------------

extern "C" {
  #include <fcntl.h>
  #include <fts.h>
  #include <libgen.h>
  #include <openssl/sha.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <sys/utsname.h>
  #include <unistd.h>
  #include <sqlite3.h>
}
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "DwmArguments.hh"
#include "DwmFreeBSDPkgManifest.hh"

using namespace std;
namespace fs = std::filesystem;

using Dwm::FreeBSDPkg::Manifest;

typedef   Dwm::Arguments<Dwm::Argument<'c',string>,
                         Dwm::Argument<'d',string>,
                         Dwm::Argument<'g',string>,
                         Dwm::Argument<'m',string>,
                         Dwm::Argument<'n',string>,
                         Dwm::Argument<'o',string>,
                         Dwm::Argument<'p',string>,
                         Dwm::Argument<'r',string>,
                         Dwm::Argument<'s',string,true>,
                         Dwm::Argument<'u',string>,
                         Dwm::Argument<'v',string>,
                         Dwm::Argument<'w',string>> MyArgType;
static MyArgType  g_args;

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void InitArgs()
{
  g_args.SetValueName<'c'>("comment");
  g_args.SetHelp<'c'>("Set the comment ('comment:') value");
  g_args.SetValueName<'d'>("desc");
  g_args.SetHelp<'d'>("Set the description ('desc:') value");
  g_args.SetValueName<'g'>("group");
  g_args.Set<'g'>("wheel");
  g_args.SetHelp<'g'>("Set the group ID of files (default is 'wheel')");
  g_args.SetValueName<'m'>("maintainer");
  g_args.SetHelp<'m'>("Set the maintainer's email address");
  g_args.SetValueName<'n'>("name");
  g_args.SetHelp<'n'>("Set the name of the package (e.g. 'libFooBar')");
  g_args.SetValueName<'o'>("origin");
  g_args.SetHelp<'o'>("Set the origin (e.g. 'devel/libFooBar')");
  g_args.SetValueName<'p'>("prefix");
  g_args.SetHelp<'p'>("Set the path where files will be installed");
  g_args.SetValueName<'r'>("manifest");
  g_args.SetHelp<'r'>("Read the given manifest file and ingest its settings");
  g_args.SetValueName<'s'>("directory");
  g_args.SetHelp<'s'>("Staging directory where files to be packaged are"
                      " located");
  g_args.Set<'u'>("root");
  g_args.SetValueName<'u'>("user");
  g_args.SetHelp<'u'>("Set the owner of files (default is 'root')");
  g_args.SetValueName<'v'>("version");
  g_args.SetHelp<'v'>("Set the version (e.g. '1.5.2')");
  g_args.SetValueName<'w'>("URL");
  g_args.SetHelp<'w'>("Set the software's official web site");
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void GetSharedLibs(const string & filename, set<string> & libs)
{
  string  lddcmd("ldd " + filename + " 2>/dev/null");
  FILE    *lddpipe = popen(lddcmd.c_str(), "r");
  if (lddpipe) {
    regex   rgx(".+[ \\t]+[=][>][ \\t]+([^ \\t]+)[ \\t]+",
                regex::ECMAScript|regex::optimize);
    smatch  sm;
    char    line[4096];
    while (fgets(line, 4096, lddpipe) != NULL) {
      string  s(line);
      if (regex_search(s, sm, rgx)) {
        if (sm.size() == 2) {
          string  lib(sm[1].str());
          libs.insert(lib);
        }
      }
    }
    pclose(lddpipe);
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static string GetInstalledPackageVersion(string packageName)
{
  string rc;
  string  query("select packages.version from packages");
  query += " where packages.name = '" + packageName + "'";
  sqlite3  *ppdb;
  if (sqlite3_open_v2("/var/db/pkg/local.sqlite", &ppdb,
                      SQLITE_OPEN_READONLY, 0)
      == SQLITE_OK) {
    sqlite3_stmt *ppStmt;
    if (sqlite3_prepare(ppdb, query.c_str(), -1, &ppStmt, 0) == SQLITE_OK) {
      if (sqlite3_step(ppStmt) == SQLITE_ROW) {
        rc = (const char *)sqlite3_column_text(ppStmt, 0);
      }
      sqlite3_finalize(ppStmt);
    }
    sqlite3_close_v2(ppdb);
  }
  return rc;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void GetPackageDeps(const set<string> & libs, set<string> & packages)
{
  string  queryPreamble("select packages.name, packages.version, packages.id,"
                        " files.package_id, files.path"
                        " from packages, files where"
                        " packages.id = files.package_id"
                        " and files.path = ");
  sqlite3  *ppdb;
  if (sqlite3_open_v2("/var/db/pkg/local.sqlite", &ppdb,
                      SQLITE_OPEN_READONLY, 0)
      == SQLITE_OK) {
    for (auto lib : libs) {
      string  qrystr(queryPreamble + '"' + lib + '"');
      sqlite3_stmt *ppStmt;
      if (sqlite3_prepare(ppdb, qrystr.c_str(), -1, &ppStmt, 0) == SQLITE_OK) {
        while (sqlite3_step(ppStmt) == SQLITE_ROW) {
          string  pkgName((const char *)sqlite3_column_text(ppStmt, 0));
          pkgName += '-';
          pkgName += (const char *)sqlite3_column_text(ppStmt, 1);
          packages.insert(pkgName);
        }
      }
      sqlite3_finalize(ppStmt);
    }
    sqlite3_close_v2(ppdb);
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void GetPackageInfo(const set<string> & pkgs,
                           vector<Manifest::Dependency> & pkginfo)
{
  regex  rgx("([^ \\t]+)[ \\t]+([^ \\t]+)[ \\t]+([^ \\t]+)\\n",
             regex::ECMAScript|regex::optimize);
  smatch  sm;
  for (auto pkg : pkgs) {
    string  querycmd("pkg query \"%n %o %v\" " + pkg);
    FILE    *querypipe = popen(querycmd.c_str(), "r");
    if (querypipe) {
      char    line[4096];
      if (fgets(line, 4096, querypipe) != NULL) {
        string  s(line);
        if (regex_search(s, sm, rgx)) {
          if (sm.size() == 4) {
            Manifest::Dependency  dep(sm[1].str(), sm[2].str(), sm[3].str());
            if (find_if(pkginfo.begin(), pkginfo.end(),
                        [&] (Manifest::Dependency const & item)
                        { return dep.Name() == item.Name(); })
                == pkginfo.end()) {
              pkginfo.push_back(dep);
            }
          }
        }
      }
      pclose(querypipe);
    }
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
vector<string> GetFiles(const string & dirName)
{
  regex              excludeRegex("^[/][#]*\\+(DESC|DISPLAY|MANIFEST|PRE_DEINSTALL|POST_DEINSTALL|PRE_INSTALL|POST_INSTALL)[~#]+");
  vector<string>     filenames;
  string             filename;
  string::size_type  idx;
  char  *dirs[2] = { strdup(dirName.c_str()), 0 };
  FTS  *fts = fts_open(&dirs[0], FTS_PHYSICAL|FTS_NOCHDIR, 0);
  if (fts) {
    FTSENT  *ftsent;
    while ((ftsent = fts_read(fts))) {
      switch (ftsent->fts_info) {
        case FTS_F:
        case FTS_SL:
          filename = ftsent->fts_path;
          idx = filename.find(dirName);
          if (idx == 0) {
            filename = filename.substr(dirName.length());
          }
          if (filename.front() != '/') {
            filename = "/" + filename;
          }
          if (! regex_match(filename, excludeRegex)) {
            filenames.push_back(filename);
          }
          break;
        default:
          break;
      }
    }
    fts_close(fts);
  }
  free(dirs[0]);
  return filenames;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static string GetSHA256(const string & filename)
{
  SHA_CTX       ctx;
  unsigned char md[SHA_DIGEST_LENGTH];
  int fd = open(filename.c_str(), O_RDONLY);
  if (fd >= 0) {
    SHA1_Init(&ctx);
    uint8_t  buf[65536];
    ssize_t  bytesRead;
    while ((bytesRead = read(fd, buf, 65536)) > 0) {
      SHA1_Update(&ctx, buf, bytesRead);
    }
    close(fd);
    SHA1_Final(&(md[0]), &ctx);
  }

  ostringstream  os;
  os << setfill('0') << hex;
  for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
    os << setw(2) << (uint16_t)md[i];
  }
  return os.str();
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
vector<Manifest::File> GetManifestFiles(const string & dirName)
{
  vector<Manifest::File>  rc;
  vector<string>  filenames = GetFiles(dirName);
  for (auto f : filenames) {
    Manifest::File  mf(f, GetSHA256(dirName + f));
    rc.push_back(mf);
  }
  return rc;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static string GetArchName()
{
  string  rc;
  struct utsname  utsn;
  
  if (uname(&utsn) == 0) {
    rc = utsn.machine;
  }
  return rc;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static string EscapeNewlinesAndQuotes(const string & s)
{
  string  rc;
  regex   rgx("\\\\");
  rc = regex_replace(s, rgx, "\\\\");
  rgx = "\"";
  rc = regex_replace(rc, rgx, "\\\"");
  rgx ="\\n";
  rc = regex_replace(rc, rgx, "\\n");
  return rc;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static string GetEscapedFileContents(const string & path)
{
  string    rc;
  ifstream  is(path.c_str());
  if (is) {
    rc = string((istreambuf_iterator<char>(is)), istreambuf_iterator<char>());
    rc = EscapeNewlinesAndQuotes(rc);
  }
  return rc;
}

//----------------------------------------------------------------------------
//!  Special files are those which we won't include in the manifest files
//!  list, but will use to populate other fields in the manifest.
//----------------------------------------------------------------------------
static bool HandleSpecialFile(const string & dirName, Manifest & manifest,
                              const Manifest::File & mf)
{
  bool  rc = false;
  typedef const string & (Manifest::*FieldSetFn)(const string & value);
  static const map<string,FieldSetFn>  fieldSetters = {
    { "/+DESC",           &Manifest::Description },
    { "/+PRE_INSTALL",    &Manifest::PreInstall },
    { "/+POST_INSTALL",   &Manifest::PostInstall },
    { "/+PRE_DEINSTALL",  &Manifest::PreDeinstall },
    { "/+POST_DEINSTALL", &Manifest::PostDeinstall }
  };
  auto  fs = fieldSetters.find(mf.Path());
  if (fs != fieldSetters.end()) {
    (manifest.*(fs->second))(GetEscapedFileContents(dirName + mf.Path()));
    rc = true;
  }
  else if (mf.Path() == "/+MANIFEST") {
    rc = true;
  }
  return rc;
}

//----------------------------------------------------------------------------
//!  For any dependencies already in the manifest... if the version in
//!  the manifest doesn't match the installed version, correct it.
//----------------------------------------------------------------------------
static void UpdatePackageDependencies(Manifest & manifest)
{
  for (auto it = manifest.Dependencies().begin();
       it != manifest.Dependencies().end(); ++it) {
    string  installedVersion =
      GetInstalledPackageVersion(it->Name());
    if ((! installedVersion.empty()) && (installedVersion != it->Version())) {
      cerr << it->Name() << " version corrected from "
           << it->Version() << " to " << installedVersion << '\n';
      it->Version(installedVersion);
    }
  }
}

//----------------------------------------------------------------------------
//!  Given the manifest and a vector of discovered installed dependencies,
//!  make the manifest dependency entries match the installed dependencies
//!  with respect to origin and version.  Also add any missing dependencies
//!  to the manifest.
//----------------------------------------------------------------------------
static void
CorrectDiscoveredDependencies(Manifest & manifest,
                              const vector<Manifest::Dependency> & discDeps)
{
  for (auto dep : discDeps) {
    auto  it =
      find_if(manifest.Dependencies().begin(),
              manifest.Dependencies().end(),
              [&] (const Manifest::Dependency & mdep) 
              {
                return ((dep.Name() == mdep.Name())
                        && ((dep.Version() != mdep.Version())
                            || (dep.Origin() != mdep.Origin())));
              });
    if (it != manifest.Dependencies().end()) {
      if (dep.Version() != it->Version()) {
        cerr << "Dependency version mismatch, " << dep.Name()
             << " version corrected to " << dep.Version() << '\n';
      }
      if (dep.Origin() != it->Origin()) {
        cerr << "Dependency origin mismatch, " << dep.Name()
             << " origin corrected to " << dep.Origin() << '\n';
      }
      *it = dep;
    }
    if (find_if(manifest.Dependencies().begin(),
                manifest.Dependencies().end(),
                [&] (const Manifest::Dependency & mdep) 
                {
                  return (dep.Name() == mdep.Name());
                })
        == manifest.Dependencies().end()) {
      cerr << "Added dependency " << dep.Name()
           << " version " << dep.Version() << '\n';
      manifest.Dependencies().push_back(dep);
    }
  }
  return;
}

//----------------------------------------------------------------------------
//!  Check for either mismatched package dependencies or missing dependencies
//!  by looking for shared libraries in files in the given directory.
//!  Patch up the dependencies if the version or origin is mismatched, add
//!  them if they're missing from the manifest.
//----------------------------------------------------------------------------
static void ScanForPackageDependencies(const string & dirName,
                                       Manifest & manifest)
{
  bool    rc = true;

  set<string>  sharedLibs;
  cerr << "Scanning " << dirName << " for dependencies\n";
  for (auto & p : fs::recursive_directory_iterator(dirName)) {
    if (fs::is_regular_file(p.path())) {
      if ((fs::status(p.path()).permissions() & fs::perms::owner_exec)
          != fs::perms::none) {
        GetSharedLibs(p.path().string(), sharedLibs);;
      }
    }
  }
  if (! sharedLibs.empty()) {
    set<string>  packageDeps;
    GetPackageDeps(sharedLibs, packageDeps);
    if (! packageDeps.empty()) {
      vector<Manifest::Dependency>  dependencies;
      GetPackageInfo(packageDeps, dependencies);
      if (! dependencies.empty()) {
        CorrectDiscoveredDependencies(manifest, dependencies);
      }
    }
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static map<char,string> ManifestFieldArgs()
{
  map<char,string>  rc;
  if (! g_args.Get<'c'>().empty())  {   rc['c'] = g_args.Get<'c'>();  }
  if (! g_args.Get<'d'>().empty())  {   rc['d'] = g_args.Get<'d'>();  }
  if (! g_args.Get<'m'>().empty())  {   rc['m'] = g_args.Get<'m'>();  }
  if (! g_args.Get<'n'>().empty())  {   rc['n'] = g_args.Get<'n'>();  }
  if (! g_args.Get<'o'>().empty())  {   rc['o'] = g_args.Get<'o'>();  }
  if (! g_args.Get<'p'>().empty())  {   rc['p'] = g_args.Get<'p'>();  }
  if (! g_args.Get<'v'>().empty())  {   rc['v'] = g_args.Get<'v'>();  }
  if (! g_args.Get<'w'>().empty())  {   rc['w'] = g_args.Get<'w'>();  }
  return rc;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
bool PopulateManifest(const string & dirName, Manifest & manifest)
{
  //  All of the fields I want to set in a Manifest object can be set
  //  with a member function with the same signature.  So I can use a
  //  map of command line options to member functions in order to set
  //  the fields.
  typedef const string & (Manifest::*FieldSetFn)(const string & value);
  static const map<char,FieldSetFn>  fieldSetters = {
    { 'n', &Manifest::Name },
    { 'v', &Manifest::Version },
    { 'o', &Manifest::Origin },
    { 'c', &Manifest::Comment },
    { 'd', &Manifest::Description },
    { 'w', &Manifest::WWW },
    { 'm', &Manifest::Maintainer },
    { 'p', &Manifest::Prefix }
  };
  
  bool  rc = false;
  vector<Manifest::File>  manifestFiles = GetManifestFiles(dirName);
  if (! manifestFiles.empty()) {
    map<char,string>  mnfstFieldArgs = ManifestFieldArgs();
    if (! mnfstFieldArgs.empty()) {
      for (auto mnfstField : mnfstFieldArgs) {
        auto  it = fieldSetters.find(mnfstField.first);
        if (it != fieldSetters.end()) {
          //  Weird syntax is due to calling a pointer to member function.
          //  (manifest.*(it->second)) is the pointer to member function.
          (manifest.*(it->second))(mnfstField.second);
        }
      }
    }
    set<string> sharedLibDeps;
    
    for (auto mfit : manifestFiles) {
      //  Only add files that are not already in the manifest.
      if (find_if(manifest.Files().begin(), manifest.Files().end(),
                  [&mfit](Manifest::File const & item)
                  { return item.Path() == mfit.Path(); })
          == manifest.Files().end()) {
        if (! HandleSpecialFile(dirName, manifest, mfit)) {
          mfit.Group(g_args.Get<'g'>());
          mfit.User(g_args.Get<'u'>());
          manifest.Files().push_back(mfit);
        }
      }
    }
    if ((! manifest.Files().empty())
        && (! manifest.Name().empty())
        && (! manifest.Version().empty())) {
      rc = true;
    }
  }
  return rc;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  InitArgs();
  int  argind = g_args.Parse(argc, argv);
  if (argind < 0) {
    cerr << g_args.Usage(argv[0], "[dependency_scan_path(s)...]");
    exit(1);
  }
  
  Dwm::FreeBSDPkg::Manifest  manifest;

  if (! g_args.Get<'r'>().empty()) {
    manifest.Parse(g_args.Get<'r'>().c_str());
  }
  
  struct stat  statbuf;
  if (stat(g_args.Get<'s'>().c_str(), &statbuf) == 0) {
    if (statbuf.st_mode & S_IFDIR) {
      //  Add files from directory argv[nextArg] to the manifest.
      if (PopulateManifest(g_args.Get<'s'>(), manifest)) {
        //  Update any dependencies that were already in the manifest, to
        //  match the installed version of the dependency.
        UpdatePackageDependencies(manifest);
        //  Scan for missing/mismatched dependencies in staging directory.
        ScanForPackageDependencies(g_args.Get<'s'>(), manifest);
        //  And then in any other directories given on the command line.
        for ( ; argind < argc; ++argind) {
          ScanForPackageDependencies(argv[argind], manifest);
        }
        //  Check for missing files.
        vector<Manifest::File>  missingFiles =
          manifest.MissingFiles(g_args.Get<'s'>());
        if (missingFiles.empty()) {
          //  No missing files.  Emit the manifest.
          cout << manifest;
          return 0;
        }
        else {
          cerr << "Missing files:\n";
          for (auto mfit : missingFiles) {
            cerr << "  " << mfit.Path() << '\n';
          }
          return 1;
        }
      }
    }
    else {
      cerr << g_args.Get<'s'>() << " is not a directory!\n";
      return 1;
    }
  }
  else {
    cerr << "Failed to stat " << g_args.Get<'s'>()
         << ": " << strerror(errno) << '\n';
    return 1;
  }

}

