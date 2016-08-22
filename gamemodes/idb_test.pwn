#define IDB_DEBUG

#include <a_samp>
#include <idb>

main() {}

public OnGameModeInit() {
    // Set up table if it doesn't exist
    new fields[][][] = {
        { "foo", "VARCHAR(32)" },
        { "bar", "INTEGER" }
    };
    idb_default("table", fields);

    // Check if there's any data in the table
    idb_all(count[1][8], "SELECT COUNT(*) FROM `table`;") {
        if(strval(count[0]) == 0) {
            // Nothing in table, set up test data
            new values[][][] = {
                { "foo", "foobar" },
                { "bar", "70" }
            };
            printf("id: %d", idb_insert("table", values));
        }
    }

    // Loop through all data, access data using column names
    idb_all_with_fields(vals[3][32], "SELECT * FROM `table`;") {
        new foo[256];
        idb_str(vals, "foo", foo);
        printf("id: %d, foo: %s, bar: %d",
            idb_int(vals, "id"),
            foo,
            idb_int(vals, "bar")
        );
    }

    // Get the first result, or execute the else{} block if nothing returned
    idb_first(vals2[3][32], "SELECT * FROM `table` WHERE `foo` = 'klhgdfg' LIMIT 1;") {
        printf("id: %s", vals2[0]);
    } else {
        print("no results");
    }

    idb_first(vals3[3][32], "SELECT * FROM `table` LIMIT 1;") {
        printf("id: %s", vals3[0]);
    } else {
        print("no results");
    }
}