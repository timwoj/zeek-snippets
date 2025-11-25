#include <stdio.h>
#include <string.h>

#include "sqlite3.h"

void test(sqlite3_stmt* stmt, sqlite3* db, double index, int add) {
    sqlite3_bind_int(stmt, 1, 2222+add);
    sqlite3_bind_int(stmt, 2, 3333+add);
    sqlite3_bind_int(stmt, 3, 3+add);
    sqlite3_bind_int(stmt, 4, 3333+add);
    sqlite3_bind_int(stmt, 5, 3+add);
    sqlite3_bind_int(stmt, 6, index);

    int step_status = sqlite3_step(stmt);
    printf("step status: %d\n", step_status);

    sqlite3_reset(stmt);
}

int main(int argc, char** argv) {
    char* err_msg = 0;
    sqlite3* db;
    int res;
    const char* put_cmd = "INSERT into testing (key, value, idx) VALUES (?, ?, ?) ON CONFLICT(key) DO UPDATE SET value=?, idx=? WHERE idx > 0 AND idx < ?";

    sqlite3_open_v2("testing.sqlite", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    sqlite3_exec(db, "CREATE TABLE testing (key INTEGER PRIMARY KEY, value INTEGER, idx INTEGER)", 0, 0, 0);

    sqlite3_stmt* put_stmt = NULL;
    res = sqlite3_prepare_v2(db, put_cmd, strlen(put_cmd), &put_stmt, NULL);
    printf("prepare result: %d\n", res);

    res = sqlite3_exec(db, "INSERT INTO testing (key, value, idx) VALUES (1111, 1111, 1)", 0, 0, &err_msg);

    printf("first insert result: %d\n", res);
    if (res != SQLITE_OK)
        printf("%s\n", err_msg);

    sqlite3_free(err_msg);
    err_msg = 0;

    res = sqlite3_exec(db, "INSERT INTO testing (key, value, idx) VALUES (2222, 2222, 2)", 0, 0, &err_msg);

    printf("second insert result: %d\n", res);
    if (res != SQLITE_OK)
        printf("%s\n", err_msg);

    sqlite3_free(err_msg);
    err_msg = 0;

    res = sqlite3_exec(db, "INSERT INTO testing (key, value, idx) VALUES (2222, 3333, 3)", 0, 0, &err_msg);
    printf("conflicting insert result: %d\n", res);
    if (res != SQLITE_OK)
        printf("%s\n", err_msg);

    sqlite3_free(err_msg);
    err_msg = 0;

    /* res = sqlite3_exec(db, "INSERT INTO testing (key, value, idx) VALUES (2222, 3333, 3)" */
    /*                        "ON CONFLICT(key) DO UPDATE SET value='3333', idx=3 WHERE idx > 0 AND idx < 2", */
    /*                    0, 0, &err_msg); */

    /* printf("ON CONFLICT insert result: %d\n", res); */
    /* printf("Number of rows changed: %d\n", sqlite3_changes(db)); */
    /* if (res != SQLITE_OK) */
    /*     printf("%s\n", err_msg); */

    /* sqlite3_free(err_msg); */
    /* err_msg = 0; */

    /* res = sqlite3_exec(db, "INSERT INTO testing (key, value, idx) VALUES (2222, 3333, 3)" */
    /*                        "ON CONFLICT(key) DO UPDATE SET value='3333', idx=3 WHERE idx > 0 AND idx <= 2", */
    /*                    0, 0, &err_msg); */

    /* printf("ON CONFLICT insert result: %d\n", res); */
    /* printf("Number of rows changed: %d\n", sqlite3_changes(db)); */
    /* if (res != SQLITE_OK) */
    /*     printf("%s\n", err_msg); */

    /* sqlite3_free(err_msg); */

    test(put_stmt, db, 2, 0);
    printf("Changes: %d\n", sqlite3_changes(db));
    test(put_stmt, db, 3, 0);
    printf("Changes: %d\n", sqlite3_changes(db));
    test(put_stmt, db, 4, 1);
    printf("Changes: %d\n", sqlite3_changes(db));

    sqlite3_close_v2(db);
}
