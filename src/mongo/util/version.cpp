// @file version.cpp

/*    Copyright 2009 10gen Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects
 *    for all of the code used other than as permitted herein. If you modify
 *    file(s) with this exception, you may extend this exception to your
 *    version of the file(s), but you are not obligated to do so. If you do not
 *    wish to do so, delete this exception statement from your version. If you
 *    delete this exception statement from all source files in the program,
 *    then also delete it in the license file.
 */

#include "mongo/platform/basic.h"

#include "mongo/util/version.h"

#include "mongo/base/parse_number.h"
#include "mongo/db/jsobj.h"
#include "mongo/util/mongoutils/str.h"

namespace mongo {

    using std::string;
    using std::stringstream;

    /* Approved formats for versionString:
     *      1.2.3
     *      1.2.3-pre-
     *      1.2.3-rc4 (rc0 up to rc49)
     *      1.2.3-rc4-pre-
     * If you really need to do something else you'll need to fix _versionArray()
     */
    const char versionString[] = "3.0.4-rc1-pre";

    // See unit test for example outputs
    BSONArray toVersionArray(const char* version){
        // this is inefficient, but cached so it doesn't matter
        BSONArrayBuilder b;
        string curPart;
        const char* c = version;
        int finalPart = 0; // 0 = final release, -100 = pre, -50 to -1 = -50 + X for rcX
        do { //walks versionString including NUL byte
            if (!(*c == '.' || *c == '-' || *c == '\0')){
                curPart += *c;
                continue;
            }

            int num;
            if ( parseNumberFromString( curPart, &num ).isOK() ) {
                b.append(num);
            }
            else if (curPart.empty()){
                verify(*c == '\0');
                break;
            }
            else if (str::startsWith(curPart, "rc")){
                int rc;
                invariantOK(parseNumberFromString( curPart.substr(2), &rc ));
                invariant(rc >= 0);
                invariant(rc < 50); // Need to adjust calculation if we pass this.
                finalPart = -50 + rc;
                break;
            }
            else if (curPart == "pre"){
                finalPart = -100;
                break;
            }

            curPart = "";
        } while (*c++);

        b.append(finalPart);
        return b.arr();
    }

    bool isSameMajorVersion( const char* version ) {

        BSONArray remoteVersionArray = toVersionArray( version );

        BSONObjIterator remoteIt(remoteVersionArray);
        BSONObjIterator myIt(versionArray);

        // Compare only the first two fields of the version
        int compareLen = 2;
        while (compareLen > 0 && remoteIt.more() && myIt.more()) {
            if (remoteIt.next().numberInt() != myIt.next().numberInt()) break;
            compareLen--;
        }

        return compareLen == 0;
    }

    const BSONArray versionArray = toVersionArray(versionString);

    string mongodVersion() {
        stringstream ss;
        ss << "db version v" << versionString;
        return ss.str();
    }

#ifndef _SCONS
    //
    // The following implementations are provided for use when buildinfo.cpp is not autogenerated.
    //

    const char * gitVersion() { return "not-scons"; }
    const char * compiledJSEngine() { return ""; }
    const char * allocator() { return ""; }
    const char * loaderFlags() { return ""; }
    const char * compilerFlags() { return ""; }

#if defined(_WIN32)

#else  // defined(_WIN32)
    string sysInfo() { return ""; }

#endif  // defined(_WIN32)
#endif  // !defined(_SCONS)

}  // namespace mongo
