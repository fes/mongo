// assert_util.cpp

/**
*    Copyright (C) 2008 10gen Inc.
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
*/

#include "stdafx.h"
#include "assert_util.h"
#include "assert.h"

namespace mongo {

	string getDbContext();
	
	Assertion lastAssert[4];
	
	/* "warning" assert -- safe to continue, so we don't throw exception. */
    void wasserted(const char *msg, const char *file, unsigned line) {
        problem() << "Assertion failure " << msg << ' ' << file << ' ' << dec << line << endl;
        sayDbContext();
        raiseError(msg && *msg ? msg : "wassertion failure");
        lastAssert[1].set(msg, getDbContext().c_str(), file, line);
    }

    void asserted(const char *msg, const char *file, unsigned line) {
        problem() << "Assertion failure " << msg << ' ' << file << ' ' << dec << line << endl;
        sayDbContext();
        raiseError(msg && *msg ? msg : "assertion failure");
        lastAssert[0].set(msg, getDbContext().c_str(), file, line);
        throw AssertionException();
    }

    void uassert_nothrow(const char *msg) {
        lastAssert[3].set(msg, getDbContext().c_str(), "", 0);
        raiseError(msg);
    }

    int uacount = 0;
    void uasserted(const char *msg) {
        if ( ++uacount < 100 )
            log() << "User Exception " << msg << endl;
        else
            RARELY log() << "User Exception " << msg << endl;
        lastAssert[3].set(msg, getDbContext().c_str(), "", 0);
        raiseError(msg);
        throw UserException(msg);
    }

    void msgasserted(const char *msg) {
        log() << "Assertion: " << msg << '\n';
        lastAssert[2].set(msg, getDbContext().c_str(), "", 0);
        raiseError(msg && *msg ? msg : "massert failure");
        throw MsgAssertionException(msg);
    }

    string Assertion::toString() {
        if ( !isSet() )
            return "";

        stringstream ss;
        ss << msg << '\n';
        if ( *context )
            ss << context << '\n';
        if ( *file )
            ss << file << ' ' << line << '\n';
        return ss.str();
    }	

}

