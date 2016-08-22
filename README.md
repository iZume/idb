# idb

Remove boiler plate around SQLite database usage for SA-MP (Pawn)

## Installation

Copy `pawno/includes/idb.inc` to your server directory

Add `#include <idb>` to the top of your gamemode/filterscript

## Config

Above `#include <idb>` you can specify compile time config options

Set the database file name:
`#define DATABASE_NAME "database.sqlite"`

Set the maximum string length for IDB to use:
`#define IDB_MAX_STRING 256`

Close the server on error:
`#define IDB_EXIT_ON_ERROR`

Enable debug mode (verbose output):
`#define IDB_DEBUG`

## Methods

### idb_query(query[])

Query the database and return a DBResult. The database is lazy loaded when idb_query is called

### idb_default(table[], fields[][][], num_fields = sizeof fields)

Create a table if it doesn't already exist. Automatically includes an `id` column which auto increments

Example:
```
new fields[][][] = {
    { "foo", "VARCHAR(32)" },
    { "bar", "INTEGER" }
};
idb_default("table", fields);
```

### idb_first(values[][], query[])

Query the database and return the first row. Acts like an if statement, so if nothing is returned you can use an else{} block.

Example:
```
idb_first(values[3][32], "SELECT * FROM `table` WHERE `foo` = 'hello world' LIMIT 1;") {
    printf("id: %s", values[0]);
} else {
    print("no results");
}
```

### idb_all(values[][], query[])

Query the database and loop through all results. Acts like a for{} or while{} loop

Example:
```
idb_all(values[2][MAX_PLAYER_NAME], "SELECT `name`, `points` FROM `users` WHERE `points` > 100;") {
    printf("%s has %s points", values[0], values[1]);
}
```

### idb_all_with_fields(values[][], query[])

Query the database and loop through all results. Gets column/field names too, so data can be accessed via the column names

Example:
```
idb_all_with_fields(values[3][32], "SELECT * FROM `table`;") {
    new foo[256];
    idb_str(values, "foo", foo);
    printf("id: %d, foo: %s", idb_int(values, "id"), foo);
}
```

### idb_str(values[][], field[], value[])

Get a string value from a database query by the column/field name (can only be used inside a `idb_all_with_fields` block)

### idb_int(values[][], field[])

As above but for an integer

For idb_int and idb_str examples see above

### idb_insert(table[], values[][][], num_fields = sizeof values)

Insert a row into a table.

Returns the id of the row, or -1 if it failed

Example:
```
new values[][][] = {
    { "foo", "foobar" },
    { "bar", "70" }
};
printf("id: %d", idb_insert("table", values));
```

## Contributing

Feel free to send a pull request with your changes
