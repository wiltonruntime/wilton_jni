import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.common.io.Files;
import org.apache.commons.io.FileUtils;
import org.junit.Test;

import java.io.File;
import java.util.LinkedHashMap;
import java.util.List;

import static net.wiltonwebtoolkit.WiltonJni.*;
import static net.wiltonwebtoolkit.WiltonJni.closeDbConnection;
import static net.wiltonwebtoolkit.WiltonJni.dbQuery;
import static org.junit.Assert.assertEquals;
import static utils.TestUtils.*;

/**
 * User: alexkasko
 * Date: 6/13/16
 */
public class DBJniTest {

    @Test
    public void testDb() throws Exception {
        File dir = null;
        try {
            dir = Files.createTempDir();
            long connectionHandle = openDbConnection("sqlite://" + dir.getAbsolutePath() + "/test.db");
            dbExecute(connectionHandle, "drop table if exists t1", "");
            dbExecute(connectionHandle, "create table t1 (foo varchar, bar int)", "");
            // insert
            dbExecute(connectionHandle, "insert into t1 values('aaa', 41)", "");
            // named params
            dbExecute(connectionHandle, "insert into t1 values(:foo, :bar)", GSON.toJson(ImmutableMap.builder()
                    .put("foo", "bbb")
                    .put("bar", 42)
                    .build()));
            // positional params
            dbExecute(connectionHandle, "insert into t1 values(?, ?)", GSON.toJson(ImmutableList.builder()
                    .add("ccc")
                    .add(43)
                    .build()));
            // select
            String json = dbQuery(connectionHandle, "select foo, bar from t1 where foo = :foo or bar = :bar order by bar",
                    GSON.toJson(ImmutableMap.builder()
                            .put("foo", "ccc")
                            .put("bar", 42)
                            .build()));
            List<LinkedHashMap<String, Object>> rs = GSON.fromJson(json, LIST_MAP_TYPE);
            assertEquals(2, rs.size());
            assertEquals("bbb", rs.get(0).get("foo"));
            // gson parsing fail, JSON int is returned correctly
            assertEquals("42", rs.get(0).get("bar"));
            assertEquals("ccc", rs.get(1).get("foo"));
            assertEquals("43", rs.get(1).get("bar"));
            closeDbConnection(connectionHandle);
        } finally {
            FileUtils.deleteDirectory(dir);
        }
    }

    @Test
    public void testDbTran() throws Exception {
        File dir = null;
        try {
            // init
            dir = Files.createTempDir();
            long connectionHandle = openDbConnection("sqlite://" + dir.getAbsolutePath() + "/test.db");
            dbExecute(connectionHandle, "drop table if exists t1", "");
            dbExecute(connectionHandle, "create table t1 (foo varchar, bar int)", "");

            // rollback
            long tran1Handle = startDbTransaction(connectionHandle);
            dbExecute(connectionHandle, "insert into t1 values(:foo, :bar)", GSON.toJson(ImmutableMap.builder()
                    .put("foo", "aaa")
                    .put("bar", 41)
                    .build()));
            rollbackDbTransaction(tran1Handle);
            // check not inserted
            String rs1Json = dbQuery(connectionHandle, "select count(*) as cc from t1", "");
            List<LinkedHashMap<String, Object>> rs1 = GSON.fromJson(rs1Json, LIST_MAP_TYPE);
            assertEquals(1, rs1.size());
            // sqlite dynamic type
            assertEquals(0, Integer.parseInt((String) rs1.get(0).get("cc")));

            // commit
            long tran2Handle = startDbTransaction(connectionHandle);
            dbExecute(connectionHandle, "insert into t1 values(:foo, :bar)", GSON.toJson(ImmutableMap.builder()
                    .put("foo", "aaa")
                    .put("bar", 41)
                    .build()));
            commitDbTransaction(tran2Handle);
            // check inserted
            String rs2Json = dbQuery(connectionHandle, "select count(*) as cc from t1", "");
            List<LinkedHashMap<String, Object>> rs2 = GSON.fromJson(rs2Json, LIST_MAP_TYPE);
            assertEquals(1, rs2.size());
            // sqlite dynamic type
            assertEquals(1, Integer.parseInt((String) rs2.get(0).get("cc")));

            closeDbConnection(connectionHandle);
        } finally {
            FileUtils.deleteDirectory(dir);
        }
    }
}
