/* 
 * File:   wiltonjs_db.cpp
 * Author: alex
 *
 * Created on August 21, 2016, 1:14 PM
 */

#include "wiltonjs/wiltonjs.hpp"

#include <functional>

#include "staticlib/serialization.hpp"

#include "wilton/wilton.h"

namespace wiltonjs {

namespace { // anonymous

namespace ss = staticlib::serialization;

const std::string EMPTY_STRING = "";

detail::handle_registry<wilton_DBConnection>& static_conn_registry() {
    static detail::handle_registry<wilton_DBConnection> registry;
    return registry;
}

detail::handle_registry<wilton_DBTransaction>& static_tran_registry() {
    static detail::handle_registry<wilton_DBTransaction> registry;
    return registry;
}

} // namespace

std::string db_connection_open(const std::string& data, void*) {
    wilton_DBConnection* conn;
    char* err = wilton_DBConnection_open(std::addressof(conn), data.c_str(), data.length());
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\ndb_connection_open error for input data: [" + data + "]"));
    int64_t handle = static_conn_registry().put(conn);
    return ss::dump_json_to_string({
        { "connectionHandle", handle}
    });
}

std::string db_connection_query(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    auto rsql = std::ref(EMPTY_STRING);
    std::string params = EMPTY_STRING;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("connectionHandle" == name) {
            handle = detail::get_json_handle(fi);
        } else if ("sql" == name) {
            rsql = detail::get_json_string(fi);
        } else if ("params" == name) {
            params = ss::dump_json_to_string(fi.value());
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'connectionHandle' not specified, data: [" + data + "]"));
    if (rsql.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'sql' not specified, data: [" + data + "]"));
    const std::string& sql = rsql.get();
    if (params.empty()) {
        params = "{}";
    }
    // get handle
    wilton_DBConnection* conn = static_conn_registry().remove(handle);
    if (nullptr == conn) throw WiltonJsException(TRACEMSG(
            "Invalid 'connectionHandle' parameter specified: [" + data + "]"));
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_DBConnection_query(conn, sql.c_str(), sql.length(),
            params.c_str(), params.length(), std::addressof(out), std::addressof(out_len));
    static_conn_registry().put(conn);
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\ndb_connection_query error for input data: [" + data + "]"));
    return detail::wrap_wilton_output(out, out_len);
}

std::string db_connection_execute(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    auto rsql = std::ref(EMPTY_STRING);
    std::string params = EMPTY_STRING;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("connectionHandle" == name) {
            handle = detail::get_json_handle(fi);
        } else if ("sql" == name) {
            rsql = detail::get_json_string(fi);
        } else if ("params" == name) {
            params = ss::dump_json_to_string(fi.value());
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'connectionHandle' not specified, data: [" + data + "]"));
    if (rsql.get().empty()) throw WiltonJsException(TRACEMSG(
            "Required parameter 'sql' not specified, data: [" + data + "]"));
    const std::string& sql = rsql.get();
    if (params.empty()) {
        params = "{}";
    }
    // get handle
    wilton_DBConnection* conn = static_conn_registry().remove(handle);
    if (nullptr == conn) throw WiltonJsException(TRACEMSG(
            "Invalid 'connectionHandle' parameter specified: [" + data + "]"));
    // call wilton
    char* err = wilton_DBConnection_execute(conn, sql.c_str(), sql.length(),
            params.c_str(), params.length());
    static_conn_registry().put(conn);
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\ndb_connection_execute error for input data: [" + data + "]"));
    return "{}";
}

std::string db_connection_close(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("connectionHandle" == name) {
            handle = detail::get_json_handle(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'connectionHandle' not specified, data: [" + data + "]"));
    // get handle
    wilton_DBConnection* conn = static_conn_registry().remove(handle);
    if (nullptr == conn) throw WiltonJsException(TRACEMSG(
            "Invalid 'connectionHandle' parameter specified: [" + data + "]"));
    // call wilton
    char* err = wilton_DBConnection_close(conn);
    if (nullptr != err) {
        static_conn_registry().put(conn);
        detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\ndb_connection_close error for input data: [" + data + "]"));
    }
    return "{}";
}

std::string db_transaction_start(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("connectionHandle" == name) {
            handle = detail::get_json_handle(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'connectionHandle' not specified, data: [" + data + "]"));
    // get handle
    wilton_DBConnection* conn = static_conn_registry().remove(handle);
    if (nullptr == conn) throw WiltonJsException(TRACEMSG(
            "Invalid 'connectionHandle' parameter specified: [" + data + "]"));
    wilton_DBTransaction* tran;
    char* err = wilton_DBTransaction_start(conn, std::addressof(tran));
    static_conn_registry().put(conn);
    if (nullptr != err) detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\ndb_transaction_start error for input data: [" + data + "]"));
    int64_t thandle = static_tran_registry().put(tran);
    return ss::dump_json_to_string({
        { "transactionHandle", thandle}
    });
}

std::string db_transaction_commit(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("transactionHandle" == name) {
            handle = detail::get_json_handle(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'transactionHandle' not specified, data: [" + data + "]"));
    // get handle
    wilton_DBTransaction* tran = static_tran_registry().remove(handle);
    if (nullptr == tran) throw WiltonJsException(TRACEMSG(
            "Invalid 'transactionHandle' parameter specified: [" + data + "]"));
    char* err = wilton_DBTransaction_commit(tran);
    if (nullptr != err) {
        static_tran_registry().put(tran);
        detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
                "\ndb_transaction_commit error for input data: [" + data + "]"));
    }
    return "{}";
}

std::string db_transaction_rollback(const std::string& data, void*) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("transactionHandle" == name) {
            handle = detail::get_json_handle(fi);
        } else {
            throw WiltonJsException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw WiltonJsException(TRACEMSG(
            "Required parameter 'transactionHandle' not specified, data: [" + data + "]"));
    // get handle
    wilton_DBTransaction* tran = static_tran_registry().remove(handle);
    if (nullptr == tran) throw WiltonJsException(TRACEMSG(
            "Invalid 'transactionHandle' parameter specified: [" + data + "]"));
    char* err = wilton_DBTransaction_rollback(tran);
    if (nullptr != err) {
        static_tran_registry().put(tran);
        detail::throw_wilton_error(err, TRACEMSG(std::string(err) +
                "\ndb_transaction_rollback error for input data: [" + data + "]"));
    }
    return "{}";
}


} // namespace
