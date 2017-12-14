import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.math.BigInteger;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

public class BuilderTest {
    static {
        try {
            System.loadLibrary("iroha");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Native code library failed to load. \n" + e);
            System.exit(1);
        }
    }

    private Keypair keys;
    private ModelBuilder builder;

    ModelBuilder base() {
        return new ModelBuilder().txCounter(BigInteger.valueOf(123))
                .createdTime(BigInteger.valueOf(System.currentTimeMillis()))
                .creatorAccountId("admin@test");
    }

    void setAddPeer() {
        builder.addPeer("123.123.123.123", keys.publicKey());
    }

    @BeforeEach
    void setUp() {
        keys = new ModelCrypto().generateKeypair();
        builder = base();
    }

    @Test
    void emptyTx() {
        ModelBuilder builder = new ModelBuilder();
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void txWithoutCommand() {
//        broken for now
//        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void outdatedAddPeer() {
        setAddPeer();
        for (BigInteger i : new BigInteger[]{BigInteger.valueOf(0),
                BigInteger.valueOf(System.currentTimeMillis() - 100_000_000),
                BigInteger.valueOf(System.currentTimeMillis() + 1000)}) {
            builder.createdTime(i);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void addPeerWithInvalidCreator() {
        setAddPeer();
        for (String i : new String[]{"", "invalid", "@invalid", "invalid@"}) {
            builder.creatorAccountId(i);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void addPeerWithInvalidKeySize() {
        setAddPeer();
        for (String i : new String[]{"", "a", "1111111111111111111111111111111", "111111111111111111111111111111111"}) {
            ModelBuilder builder = base().addPeer("123.123.123.123", new PublicKey(i));
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void addPeerWithInvalidHost() {
        setAddPeer();
        for (String i : new String[]{"257.257.257.257", "host#host", "asd@asd", "ab..cd"}) {
            ModelBuilder builder = base().addPeer(i, keys.publicKey());
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    long protoSize(UnsignedTx tx) {
        return new ModelTransactionProto().signAndAddSignature(tx, keys).size();
    }

    @Test
    void keygen() {
        assertEquals(keys.publicKey().size(), 32);
        assertEquals(keys.privateKey().size(), 64);
    }

    @Test
    void addPeer() {
        UnsignedTx tx = builder.addPeer("123.123.123.123", keys.publicKey()).build();
        assertEquals(protoSize(tx), 180);
    }

    @Test
    void addSignatory() {
        UnsignedTx tx = builder.addSignatory("admin@test", keys.publicKey()).build();
        assertEquals(protoSize(tx), 175);
    }

    @Test
    void addAssetQuantity() {
        UnsignedTx tx = builder.addAssetQuantity("admin@test", "asset#domain", "12.345").build();
        assertEquals(protoSize(tx), 164);
    }

    @Test
    void removeSignatory() {
        UnsignedTx tx = builder.removeSignatory("admin@test", keys.publicKey()).build();
        assertEquals(protoSize(tx), 175);
    }

    @Test
    void createAccount() {
        UnsignedTx tx = builder.createAccount("admin", "domain", keys.publicKey()).build();
        assertEquals(protoSize(tx), 178);
    }

    @Test
    void createDomain() {
        UnsignedTx tx = builder.createDomain("domain", "role").build();
        assertEquals(protoSize(tx), 143);
    }

    @Test
    void setAccountQuorum() {
        UnsignedTx tx = builder.setAccountQuorum("admin@test", 123).build();
        assertEquals(protoSize(tx), 143);
    }

    @Test
    void transferAsset() {
        UnsignedTx tx = builder.transferAsset("from@test", "to@test", "asset#test", "description", "123.456").build();
        assertEquals(protoSize(tx), 184);
    }
}
