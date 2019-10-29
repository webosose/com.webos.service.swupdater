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

#ifndef UTIL_UTIL_H_
#define UTIL_UTIL_H_

#include <iostream>
#include <glib.h>

using namespace std;

class Util {
public:
    Util() {}
    virtual ~Util() {}

    static bool isFileExist(const string& filename);
    static bool touchFile(const string& filename);
    static bool removeFile(const string& filename);
    static bool writeFile(const string& filename, const string& contents);
    static bool makeDir(const string& dir);
    static bool reboot();

    static string sha1(const string& filename);

    // Call function asynchronously
    template <typename T>
    static bool async(T function, guint timeout = 0)
    {
        AsyncCall<T> *p = new AsyncCall<T>(function);
        g_timeout_add(timeout, cbAsync, (gpointer)p);
        return true;
    }

private:
    // abstract class for async call
    class IAsyncCall {
    public:
        virtual ~IAsyncCall() { }
        virtual void Call() = 0;
    };

    // implementaion for async call
    template <typename T>
    class AsyncCall : public IAsyncCall {
    public:
        AsyncCall(T _func) : func(_func) {}

        void Call() { func(); }
    private:
        T func;
    };

    //! It's called when get response async call
    static gboolean cbAsync(gpointer data);
};

#endif /* UTIL_FILEUTIL_H_ */
