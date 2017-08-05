import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.common.io.Files;
import org.junit.BeforeClass;
import org.junit.Test;
import utils.TestGateway;

import java.io.File;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import static net.wiltontoolkit.WiltonJni.*;
import static org.junit.Assert.assertEquals;
import static utils.TestUtils.*;

/**
 * User: alexkasko
 * Date: 6/13/16
 */
public class DBJniTest {

    @BeforeClass
    public static void init() {
        // init, no logging by default, enable it when needed
        initWiltonOnce(new TestGateway(), LOGGING_DISABLE, getJsDir().getAbsolutePath());
    }

    @Test
    public void testDb() throws Exception {
        File dir = null;
        try {
            dir = Files.createTempDir();
            String out = wiltoncall("db_connection_open", "sqlite://" + dir.getAbsolutePath() + "/test.db");
            Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
            long connectionHandle = hamap.get("connectionHandle");
            wiltoncall("db_connection_execute", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .put("sql", "drop table if exists t1")
                    .build()));
            wiltoncall("db_connection_execute", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .put("sql", "create table t1 (foo varchar, bar int)")
                    .build()));
            // insert
            wiltoncall("db_connection_execute", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .put("sql", "insert into t1 values('aaa', 41)")
                    .build()));
            // named params
            wiltoncall("db_connection_execute", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .put("sql", "insert into t1 values(:foo, :bar)")
                    .put("params", ImmutableMap.builder()
                            .put("foo", "bbb")
                            .put("bar", 42)
                            .build())
                    .build()));
            // positional params
            wiltoncall("db_connection_execute", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .put("sql", "insert into t1 values(?, ?)")
                    .put("params", ImmutableList.builder()
                            .add("ccc")
                            .add(43)
                            .build())
                    .build()));
            // select
            String json = wiltoncall("db_connection_query", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .put("sql", "select foo, bar from t1 where foo = :foo or bar = :bar order by bar")
                    .put("params", ImmutableMap.builder()
                            .put("foo", "ccc")
                            .put("bar", 42)
                            .build())
                    .build()));
            List<LinkedHashMap<String, Object>> rs = GSON.fromJson(json, LIST_MAP_TYPE);
            assertEquals(2, rs.size());
            assertEquals("bbb", rs.get(0).get("foo"));
            // gson parsing fail, JSON int is returned correctly
            assertEquals("42", rs.get(0).get("bar"));
            assertEquals("ccc", rs.get(1).get("foo"));
            assertEquals("43", rs.get(1).get("bar"));
            wiltoncall("db_connection_close", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .build()));
        } finally {
            deleteDirQuietly(dir);
        }
    }

    @Test
    public void testDbTran() throws Exception {
        File dir = null;
        try {
            // init
            dir = Files.createTempDir();
            String out = wiltoncall("db_connection_open", "sqlite://" + dir.getAbsolutePath() + "/test.db");
            Map<String, Long> hamap = GSON.fromJson(out, LONG_MAP_TYPE);
            long connectionHandle = hamap.get("connectionHandle");
            wiltoncall("db_connection_execute", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .put("sql", "drop table if exists t1")
                    .build()));
            wiltoncall("db_connection_execute", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .put("sql", "create table t1 (foo varchar, bar int)")
                    .build()));

            // rollback
            String tran1out = wiltoncall("db_transaction_start", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .build()));
            Map<String, Long> tran1hamap = GSON.fromJson(tran1out, LONG_MAP_TYPE);
            long tran1Handle = tran1hamap.get("transactionHandle");
            wiltoncall("db_connection_execute", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .put("sql", "insert into t1 values(:foo, :bar)")
                    .put("params", ImmutableMap.builder()
                            .put("foo", "aaa")
                            .put("bar", 41)
                            .build())
                    .build()));
            wiltoncall("db_transaction_rollback", GSON.toJson(ImmutableMap.builder()
                    .put("transactionHandle", tran1Handle)
                    .build()));

            // check not inserted
            String rs1Json = wiltoncall("db_connection_query", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .put("sql", "select count(*) as cc from t1")
                    .build()));
            List<LinkedHashMap<String, Object>> rs1 = GSON.fromJson(rs1Json, LIST_MAP_TYPE);
            assertEquals(1, rs1.size());
            // sqlite dynamic type
            assertEquals(0, Integer.parseInt((String) rs1.get(0).get("cc")));

            // commit
            String tran2out = wiltoncall("db_transaction_start", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .build()));
            Map<String, Long> tran2hamap = GSON.fromJson(tran2out, LONG_MAP_TYPE);
            long tran2Handle = tran2hamap.get("transactionHandle");
            wiltoncall("db_connection_execute", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .put("sql", "insert into t1 values(:foo, :bar)")
                    .put("params", ImmutableMap.builder()
                            .put("foo", "aaa")
                            .put("bar", 41)
                            .build())
                    .build()));
            wiltoncall("db_transaction_commit", GSON.toJson(ImmutableMap.builder()
                    .put("transactionHandle", tran2Handle)
                    .build()));

            // check inserted
            String rs2Json = wiltoncall("db_connection_query", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .put("sql", "select count(*) as cc from t1")
                    .build()));
            List<LinkedHashMap<String, Object>> rs2 = GSON.fromJson(rs2Json, LIST_MAP_TYPE);
            assertEquals(1, rs2.size());
            // sqlite dynamic type
            assertEquals(1, Integer.parseInt((String) rs2.get(0).get("cc")));

            wiltoncall("db_connection_close", GSON.toJson(ImmutableMap.builder()
                    .put("connectionHandle", connectionHandle)
                    .build()));
        } finally {
            deleteDirQuietly(dir);
        }
    }
}
