import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.math.BigInteger;

import iroha.protocol.Commands;
import iroha.protocol.BlockOuterClass;
import com.google.protobuf.InvalidProtocolBufferException;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

// TODO luckychess 8.08.2018 add test for number of methods
// in interface and proto implementation IR-1080

public class BuilderTest {
    static {
        try {
            System.loadLibrary("irohajava");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Native code library failed to load. \n" + e);
            System.exit(1);
        }
    }

    private Keypair keys;
    private ModelTransactionBuilder builder;

    ModelTransactionBuilder base() {
        return new ModelTransactionBuilder()
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
        ModelTransactionBuilder builder = new ModelTransactionBuilder();
        assertThrows(IllegalArgumentException.class, builder::build);
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
            ModelTransactionBuilder builder = base().addPeer("123.123.123.123", new PublicKey(i));
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void addPeerWithInvalidHost() {
        setAddPeer();
        for (String i : new String[]{"257.257.257.257", "host#host", "asd@asd", "ab..cd"}) {
            ModelTransactionBuilder builder = base().addPeer(i, keys.publicKey());
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    Blob proto(UnsignedTx tx) {
        return new ModelProtoTransaction().signAndAddSignature(tx, keys);
    }

    /**
     * Performs check that Blob contains valid proto Transaction
     * @param serialized blob with binary data for check
     * @return true if valid
     */
    private boolean checkProtoTx(Blob serialized) {
        ByteVector blob = serialized.blob();
        byte bs[] = new byte[(int)blob.size()];

        for (int i = 0; i < blob.size(); i++) {
            bs[i] = (byte)blob.get(i);
        }

        try {
            BlockOuterClass.Transaction.parseFrom(bs);
        } catch (InvalidProtocolBufferException e) {
            System.out.print("Exception: ");
            System.out.println(e.getMessage());
            return false;
        }
        return true;
    }

    @Test
    void addPeer() {
        UnsignedTx tx = builder.addPeer("123.123.123.123:123", keys.publicKey()).build();
        assertTrue(checkProtoTx(proto(tx)));
    }

    @Test
    void addSignatory() {
        UnsignedTx tx = builder.addSignatory("admin@test", keys.publicKey()).build();
        assertTrue(checkProtoTx(proto(tx)));
    }

    @Test
    void addAssetQuantity() {
        UnsignedTx tx = builder.addAssetQuantity("admin@test", "asset#domain", "12.345").build();
        assertTrue(checkProtoTx(proto(tx)));
    }

    @Test
    void removeSignatory() {
        UnsignedTx tx = builder.removeSignatory("admin@test", keys.publicKey()).build();
        assertTrue(checkProtoTx(proto(tx)));
    }

    @Test
    void createAccount() {
        UnsignedTx tx = builder.createAccount("admin", "domain", keys.publicKey()).build();
        assertTrue(checkProtoTx(proto(tx)));
    }

    @Test
    void createDomain() {
        UnsignedTx tx = builder.createDomain("domain", "role").build();
        assertTrue(checkProtoTx(proto(tx)));
    }

    @Test
    void setAccountQuorum() {
        UnsignedTx tx = builder.setAccountQuorum("admin@test", 123).build();
        assertTrue(checkProtoTx(proto(tx)));
    }

    @Test
    void transferAsset() {
        UnsignedTx tx = builder.transferAsset("from@test", "to@test", "asset#test", "description", "123.456").build();
        assertTrue(checkProtoTx(proto(tx)));
    }

    @Test
    void setAccountDetail() {
        UnsignedTx tx = builder.setAccountDetail("admin@test", "fyodor", "kek").build();
        assertTrue(checkProtoTx(proto(tx)));
    }
}
