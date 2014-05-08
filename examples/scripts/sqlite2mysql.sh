#!/bin/sed -f
#
# Input: sqlite3 .dump format
# Output: MySQL input format
#
/BEGIN TRANSACTION;/d
/interval INTEGER NOT NULL/s/interval/arcInt/
/dateTime INTEGER NOT NULL UNIQUE PRIMARY KEY/s/UNIQUE //
s/"/`/g
/COMMIT;/d
