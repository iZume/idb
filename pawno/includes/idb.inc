#if !defined DATABASE_NAME
    #define DATABASE_NAME "database.sqlite"
#endif

#if !defined IDB_MAX_STRING
    #define IDB_MAX_STRING 256
#endif

#if defined IDB_EXIT_ON_ERROR
    #define idb_error(%0) printf("IDB Error: " %0); SendRconCommand("Exit")
#else
    #define idb_error(%0) printf("IDB Error: " %0)
#endif

#if defined IDB_DEBUG
    #define idb_debug(%0) printf("IDB Debug: " %0)
#else
    #define idb_debug(%0);
#endif

#define idb_check_result(%0); if(%0 == DBResult:0) { idb_error("Query failed"); }

/**
 The database being used
 */
new DB:_idb;

/**
 Open the database
 
 There is no need to call this, the database is lazy loaded when the first
 query is executed
 */
stock DB:idb() {
    if(_idb == DB:0) {
        _idb = db_open(DATABASE_NAME);
        if(_idb == DB:0) {
            idb_error("could not open database (" DATABASE_NAME ")");
        } else {
            idb_debug("opened database (" DATABASE_NAME ")");
        }
    }
    return _idb;
}

/**
 Forcibly close the database
 
 This is ran automatically after OnGameModeExit or OnFilterScriptExit. You
 can also call it explicitly if you need to open the database outside of IDB,
 as it will be lazy loaded again when needed.
 */
stock idb_close() {
    if(_idb != DB:0) {
        db_close(_idb);
        _idb = DB:0;
        idb_debug("closed database (" DATABASE_NAME ")");
    }
}

public OnGameModeExit() {
    CallLocalFunction("IDB_OnGameModeExit", "");
    idb_close();
}
#if defined _ALS_OnGameModeExit
    #undef OnGameModeExit
#else
    #define _ALS_OnGameModeExit
#endif
#define OnGameModeExit IDB_OnGameModeExit
forward IDB_OnGameModeExit();

public OnFilterScriptExit() {
    CallLocalFunction("IDB_OnFilterScriptExit", "");
    idb_close();
}
#if defined _ALS_OnFilterScriptExit
    #undef OnFilterScriptExit
#else
    #define _ALS_OnFilterScriptExit
#endif
#define OnFilterScriptExit IDB_OnFilterScriptExit
forward IDB_OnFilterScriptExit();

/**
 Perform a query on the database and return a DBResult
 */
stock DBResult:idb_query(query[]) {
    idb_debug("%s", query);
    #if defined IDB_DEBUG
        new DBResult:result = db_query(idb(), query);
        idb_debug("Result: %d", _:result);
        return result;
    #else
        return db_query(idb(), query);
    #endif
}

/**
 Call this in OnGameModeInit to specify your database structure
 
 It will automatically add an auto incremented `id` column
 
 If you wish to change an existing table structure you will have to migrate
 it yourself
 */
stock bool:idb_default(
    name[],
    fields[][][],
    num_fields = sizeof fields
) {
    idb_debug("Default table: %s", name);
    new query[IDB_MAX_STRING];
    format(
        query,
        sizeof query,
        "CREATE TABLE IF NOT EXISTS `%q` ( `id` INTEGER PRIMARY KEY NOT NULL",
        name
    );
    for(new i = 0; i < num_fields; i++) {
        format(
            query,
            sizeof query,
            "%s, `%q` %s",
            query,
            fields[i][0],
            fields[i][1]
        );
    }
    format(query, sizeof query, "%s );", query);
    new DBResult:result = idb_query(query);
    if(result == DBResult:0) {
        idb_error("could not create table (%s)", name);
        return false;
    }
    db_free_result(result);
    return true;
}

/**
 Private API, never use
 */
stock bool:_idb_set_values(
    DBResult:result,
    row,
    row_count,
    field_count,
    values[][],
    max_fields,
    max_string
) {
    idb_check_result(result);
    if(row < row_count) {
        for(new f = 0; f < field_count; f++) {
            if(f == max_fields) {
                idb_error("more fields returned (%d) than space in array (%d)", field_count, max_fields);
                break;
            }
            db_get_field(result, f, values[f], max_string);
        }

        db_next_row(result);
        return true;
    }
    db_free_result(result);
    return false;
}

stock bool:_idb_set_values_with_fields(
    DBResult:result,
    row,
    row_count,
    field_count,
    values[][],
    max_values,
    max_string,
    &bool:set_fields,
    fields[][],
    max_fields,
    max_field_string
) {
    if(!set_fields) {
        idb_check_result(result);
        set_fields = true;
        for(new f = 0; f < field_count; f++) {
            if(f == max_fields) {
                idb_error("more fields returned (%d) than space in array (%d)", field_count, max_fields);
                break;
            }
            db_field_name(result, f, fields[f], max_field_string);
        }
    }
    return _idb_set_values(
        result, row, row_count, field_count, values, max_values, max_string);
}

stock bool:_idb_first(query[], values[][], max_values, max_string) {
    new DBResult:result = idb_query(query);
    if(result == DBResult:0) return false;
    if(db_num_rows(result) == 0) {
        db_free_result(result);
        return false;
    }
    _idb_set_values(
        result,
        0,
        1,
        db_num_fields(result),
        values,
        max_values,
        max_string
    );
    db_free_result(result);
    return true;
}

/**
 Query the database and get the first result

 Usage:
     idb_first(my_data[NUM_FIELDS][MAX_STRING], "SELECT * FROM `table` LIMIT 1;") {
         printf("col1: %s", my_data[0]);
         printf("col2: %s", my_data[1]);
     } else {
         print("no results");
     }
 */
#define idb_first(%0[%1][%2],%3) \
    new %0[%1][%2]; \
    if(_idb_first(%3, %0, %1, %2))

/**
 Query the database and loop through the results

 Usage:
     idb_all(my_data[NUM_FIELDS][MAX_STRING], "SELECT * FROM `table`;") {
         printf("col1: %s", my_data[0]);
         printf("col2: %s", my_data[1]);
     }
 */
#define idb_all(%0[%1][%2],%3) \
    for( \
        new DBResult:_idb_r_%0_ = idb_query(%3), \
            _idb_rr_%0_     = db_num_rows(_idb_r_%0_), \
            _idb_nf_%0_     = db_num_fields(_idb_r_%0_), \
            _idb_cr_%0_      = 0, \
            %0[%1][%2]; \
        _idb_set_values( \
            _idb_r_%0_, \
            _idb_cr_%0_, \
            _idb_rr_%0_, \
            _idb_nf_%0_, \
            %0, \
            %1, \
            %2 \
        ); \
        _idb_cr_%0_++ \
    )

/**
 Query the database and loop through the results, and access them using the
 column names

 Usage:
     idb_all_with_fields(my_data[NUM_FIELDS][MAX_STRING], "SELECT * FROM `table`;") {
         new foo[MAX_STRING];
         idb_str(my_data, "foo", foo);
         printf("foo: %s", foo);
         printf("bar: %d", idb_int(my_data, "bar"));
     }
 */
#define idb_all_with_fields(%0[%1][%2],%6) \
    for( \
        new DBResult:_idb_r_%0_ = idb_query(%6), \
            _idb_rr_%0_     = db_num_rows(_idb_r_%0_), \
            _idb_nf_%0_      = db_num_fields(_idb_r_%0_), \
            _idb_cr_%0_      = 0, \
            %0[%1][%2], \
            _idb_f_%0[%1][%2], \
            bool:_idb_gf_%0_ = false; \
        _idb_set_values_with_fields( \
            _idb_r_%0_, \
            _idb_cr_%0_, \
            _idb_rr_%0_, \
            _idb_nf_%0_, \
            %0, \
            %1, \
            %2, \
            _idb_gf_%0_, \
            _idb_f_%0, \
            %1, \
            %2 \
        ); \
        _idb_cr_%0_++ \
    )

stock bool:_idb_get_field_val(
    values[][],
    fields[][],
    field[],
    value[],
    fields_length = sizeof fields,
    max_string = sizeof value
) {
    for(new i = 0; i < fields_length; i++) {
        if(fields[i][0] != 0 && strcmp(fields[i], field) == 0) {
            new len = strlen(values[i]);
            if(len >= max_string) {
                idb_error("length of value (%d) for field %s exceeds size of value string (%d)", len, field, max_string);
                len = max_string - 1;
            }
            for(new s = 0; s < len; s++) {
                value[s] = values[i][s];
            }
            value[len] = 0;
            return true;
        }
    }
    idb_error("field not found in query: %s", field);
    return false;
}

stock _idb_get_field_val_int(
    values[][],
    fields[][],
    field[],
    fields_length = sizeof fields
) {
    new value[IDB_MAX_STRING];
    _idb_get_field_val(values, fields, field, value, fields_length);
    return strval(value);
}

#define idb_str(%0,%1,%2) _idb_get_field_val(%0,_idb_f_%0,%1,%2)
#define idb_int(%0,%1)    _idb_get_field_val_int(%0,_idb_f_%0,%1)

/**
 Insert a row into a table and return the row ID
 */
stock idb_insert(
    table[],
    values[][][],
    num_fields = sizeof values
) {
    new query[IDB_MAX_STRING];
    format(
        query,
        sizeof query,
        "INSERT INTO `%q` (",
        table
    );
    for(new i = 0; i < num_fields; i++) {
        format(
            query,
            sizeof query,
            "%s%s`%q`",
            query,
            (i == 0 ? ("") : ", "),
            values[i][0]
        );
    }
    format(query, sizeof query, "%s) VALUES (", query);
    for(new i = 0; i < num_fields; i++) {
        format(
            query,
            sizeof query,
            "%s%s'%q'",
            query,
            (i == 0 ? ("") : ", "),
            values[i][1]
        );
    }
    format(query, sizeof query, "%s);", query);
    new DBResult:result = idb_query(query);
    if(result == DBResult:0) {
        idb_error("could not insert data");
        return -1;
    }
    db_free_result(result);
    idb_all(insert_id[1][8], "SELECT last_insert_rowid();") {
        return strval(insert_id[0]);
    }
    idb_error("could not retrieve row id");
    return -1;
}
