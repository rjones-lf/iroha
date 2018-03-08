import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.math.BigInteger;

import iroha.protocol.BlockOuterClass;
import iroha.protocol.Queries;
import com.google.protobuf.InvalidProtocolBufferException;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

// TODO luckychess 8.08.2018 add test for number of methods
// in interface and proto implementation IR-1080

public class QueryTest {
    static {
        try {
            System.loadLibrary("irohajava");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Native code library failed to load. \n" + e);
            System.exit(1);
        }
    }

    private Keypair keys;
    private ModelQueryBuilder builder;

    ModelQueryBuilder base() {
        return new ModelQueryBuilder().queryCounter(BigInteger.valueOf(123))
                .createdTime(BigInteger.valueOf(System.currentTimeMillis()))
                .creatorAccountId("admin@test");
    }

    void setGetAccount() {
        builder.getAccount("user@test");
    }

    @BeforeEach
    void setUp() {
        keys = new ModelCrypto().generateKeypair();
        builder = base();
    }

    @Test
    void emptyQuery() {
        ModelQueryBuilder builder = new ModelQueryBuilder();
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void outdatedAddPeer() {
        setGetAccount();
        for (BigInteger i : new BigInteger[]{BigInteger.valueOf(0),
                BigInteger.valueOf(System.currentTimeMillis() - 100_000_000),
                BigInteger.valueOf(System.currentTimeMillis() + 1000)}) {
            builder.createdTime(i);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountWithInvalidCreator() {
        setGetAccount();
        for (String i : new String[]{"", "invalid", "@invalid", "invalid@"}) {
            builder.creatorAccountId(i);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    Blob proto(UnsignedQuery query) {
        return new ModelProtoQuery().signAndAddSignature(query, keys);
    }

    /**
     * Performs check that Blob contains valid proto Query
     * @param serialized blob with binary data for check
     * @return true if valid
     */
    private <T> boolean checkProtoQuery(Blob serialized) {
        ByteVector blob = serialized.blob();
        byte bs[] = new byte[(int)blob.size()];

        for (int i = 0; i < blob.size(); i++) {
            bs[i] = (byte)blob.get(i);
        }

        try {
            Queries.Query.parseFrom(bs);
        } catch (InvalidProtocolBufferException e) {
            System.out.print("Exception: ");
            System.out.println(e.getMessage());
            return false;
        }
        return true;
    }

    @Test
    void getAccount() {
        UnsignedQuery query = builder.getAccount("user@test").build();
        assertTrue(checkProtoQuery(proto(query)));
    }

    @Test
    void getSignatories() {
        UnsignedQuery query = builder.getSignatories("user@test").build();
        assertTrue(checkProtoQuery(proto(query)));
    }

    @Test
    void getAccountTransactions() {
        UnsignedQuery query = builder.getAccountTransactions("user@test").build();
        assertTrue(checkProtoQuery(proto(query)));
    }

    @Test
    void getAccountAssetTransactions() {
        UnsignedQuery query = builder.getAccountAssetTransactions("user@test", "coin#test").build();
        assertTrue(checkProtoQuery(proto(query)));
    }

    @Test
    void getAccountAssets() {
        UnsignedQuery query = builder.getAccountAssets("user@test", "coin#test").build();
        assertTrue(checkProtoQuery(proto(query)));
    }

    @Test
    void getRoles() {
        UnsignedQuery query = builder.getRoles().build();
        assertTrue(checkProtoQuery(proto(query)));
    }

    @Test
    void getAssetInfo() {
        UnsignedQuery query = builder.getAssetInfo("coin#test").build();
        assertTrue(checkProtoQuery(proto(query)));
    }

    @Test
    void getRolePermissions() {
        UnsignedQuery query = builder.getRolePermissions("user").build();
        assertTrue(checkProtoQuery(proto(query)));
    }

    @Test
    void getTransactions() {
        HashVector hv = new HashVector();
        char[] hash = new char[32];
        java.util.Arrays.fill(hash, '1');
        hv.add(new Hash(new String(hash)));
        java.util.Arrays.fill(hash, '2');
        hv.add(new Hash(new String(hash)));
        UnsignedQuery query = builder.getTransactions(hv).build();
        assertTrue(checkProtoQuery(proto(query)));
    }

    @Test
    void getAccountDetail() {
        UnsignedQuery query = builder.getAccountDetail("user@test", "hello").build();
        assertTrue(checkProtoQuery(proto(query)));
    }
}
