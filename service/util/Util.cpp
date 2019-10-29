// Copyright (c) 2019 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "util/Util.h"

#include <boost/version.hpp>
#if (BOOST_VERSION >= 106800)
#include <boost/uuid/detail/sha1.hpp>
#else
#include <boost/uuid/sha1.hpp>
#endif
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>

bool Util::isFileExist(const string& filename)
{
    return access(filename.c_str(), F_OK) == 0;
}

bool Util::touchFile(const string& filename)
{
    if (isFileExist(filename)) {
        return true;
    }

    FILE *file = fopen(filename.c_str(), "w");
    if (file == NULL) {
        return false;
    }
    fclose(file);
    return true;
}

bool Util::removeFile(const string& filename)
{
    return remove(filename.c_str()) == 0;
}

bool Util::writeFile(const string& filename, const string& contents)
{
    ofstream file(filename);
    if (!file.good()) {
        return false;
    }
    file << contents;
    return true;
}

bool Util::makeDir(const string& dir)
{
    std::string command = "mkdir -p " + dir;
    int rc = ::system(command.c_str());
    return WIFEXITED(rc) && WEXITSTATUS(rc) == 0;
}

bool Util::reboot()
{
    int rc = ::system("reboot");
    return WIFEXITED(rc) && WEXITSTATUS(rc) == 0;
}

string Util::sha1(const string& filename)
{
    ifstream file(filename);
    if (!file.good()) {
        return "";
    }

    stringstream ss;
    boost::uuids::detail::sha1 sha1;
    unsigned int digest[5];
    char buf[4096];
    while (file.good()) {
        file.read(buf, sizeof(buf));
        sha1.process_bytes(buf, file.gcount());
    }
    sha1.get_digest(digest);
    for (int i = 0; i < 5; i++) {
        cout << std::hex;
        ss << std::hex << std::setfill('0') << std::setw(8) << digest[i];
    }
    return ss.str();
}

gboolean Util::cbAsync(gpointer data)
{
    IAsyncCall *p = reinterpret_cast<IAsyncCall*>(data);
    if (!p) return false;

    p->Call();

    delete p;

    return false;
}
