import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.Disabled;

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

    // Symbols of type 1 (format [a-z_0-9]{1,32}) are used
    // as account_name, asset_name and role_id.
    private final String[] validNameSymbols1 = {
        "a",
        "asset",
        "234234",
        "_",
        "_123",
        "123_23",
        "234asset_",
        "__",
        "12345678901234567890123456789012"
    };
    private final String[] invalidNameSymbols1 = {
        "",
        "A",
        "assetV",
        "asSet",
        "asset%",
        "^123",
        "verylongassetname_thenameislonger",
        "verylongassetname_thenameislongerthanitshouldbe",
        "assset-01"
    };

    // Symbols of type 2 (format [A-Za-z0-9_]{1,64})
    // are used as key identifier for setAccountDetail command
    private final String[] validNameSymbols2 = {
        "a",
        "A",
        "1",
        "_",
        "Key",
        "Key0_",
        "verylongAndValidKeyName___1110100010___veryveryveryverylongvalid"
    };
    private final String[] invalidNameSymbols2 = {
        "",
        "Key&",
        "key-30",
        "verylongAndValidKeyName___1110100010___veryveryveryverylongvalid1",
        "@@@"
    };

    private final String[] invalidHosts = {
        "257.257.257.257",
        "host#host",
        "asd@asd",
        "ab..cd",
        "",
        "   "
    };

    private final String[] invalidKeysBytes = {
        "",
        "a",
        "1111111111111111111111111111111",
        "111111111111111111111111111111111"
    };

    ModelQueryBuilder base() {
        return new ModelQueryBuilder().queryCounter(BigInteger.valueOf(123))
                .createdTime(BigInteger.valueOf(System.currentTimeMillis()))
                .creatorAccountId("admin@test");
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

    /* ====================== GetAccount Tests ====================== */


    @Test
    void outdatedGetAccount() {
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
        for (String accountName: invalidNameSymbols1) {
            ModelQueryBuilder builder = base().creatorAccountId(accountName + "@test");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountWithInvalidCreatorDomain() {
        setGetAccount();
        for (String host: invalidHosts) {
            ModelQueryBuilder builder = base().creatorAccountId("user@" + host);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountWithEmptyCreator() {
        setGetAccount();
        builder.creatorAccountId("");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void getAccount() {
        for (String accountName: validNameSymbols1) {
            UnsignedQuery query = builder.getAccount(accountName + "@test").build();
            assertTrue(checkProtoQuery(proto(query)));
        }
    }

    @Test
    void getAccountWithInvalidName() {
        for (String accountName: invalidNameSymbols1) {
            ModelQueryBuilder builder = base().getAccount(accountName + "@test");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountWithInvalidDomain() {
        for (String host: invalidHosts) {
            ModelQueryBuilder builder = base().getAccount("admin@" + host);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    /* ====================== GetSignatories Tests ====================== */

    @Test
    void getSignatories() {
        for (String account: validNameSymbols1) {
            UnsignedQuery query = builder.getSignatories(account + "@test").build();
            assertTrue(checkProtoQuery(proto(query)));
        }
    }

    @Test
    void getSignatoriesInvalidAccount() {
        for (String account: invalidNameSymbols1) {
            ModelQueryBuilder builder = base().getSignatories(account + "@test");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getSignatoriesEmptyAccount() {
        builder.getSignatories("");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void getSignatoriesInvalidDomain() {
        for (String host: invalidHosts) {
            ModelQueryBuilder builder = base().getSignatories("user@" + host);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    /* ====================== GetAccountTransactions Tests ====================== */

    @Test
    void getAccountTransactions() {
        for (String account: validNameSymbols1) {
            UnsignedQuery query = builder.getAccountTransactions(account + "@test").build();
            assertTrue(checkProtoQuery(proto(query)));
        }
    }

    @Test
    void getAccountTransactionsInvalidName() {
        for (String account: invalidNameSymbols1) {
            ModelQueryBuilder builder = base().getAccountTransactions(account + "@test");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountTransactionsInvalidDomain() {
        for (String host: invalidHosts) {
            ModelQueryBuilder builder = base().getAccountTransactions("user@" + host);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountTransactionsEmptyAccount() {
        ModelQueryBuilder builder = base().getAccountTransactions("");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    /* ====================== GetAccountAssetTransactions Tests ====================== */

    @Test
    void getAccountAssetTransactions() {
        for (String name: validNameSymbols1) {
            UnsignedQuery query = builder.getAccountAssetTransactions(name + "@test", name + "#test").build();
            assertTrue(checkProtoQuery(proto(query)));
        }
    }

    @Test
    void getAccountAssetTransactionsInvalidAccountName() {
        for (String account: invalidNameSymbols1) {
            ModelQueryBuilder builder = base().getAccountAssetTransactions(account + "@test", "coin#test");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountAssetTransactionsInvalidAccountDomain() {
        for (String host: invalidHosts) {
            ModelQueryBuilder builder = base().getAccountAssetTransactions("user@" + host, "coin#test");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountAssetTransactionsEmptyAccount() {
        ModelQueryBuilder builder = base().getAccountAssetTransactions("", "coin#test");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void getAccountAssetTransactionsInvalidAssetName() {
        for (String asset: invalidNameSymbols1) {
            ModelQueryBuilder builder = base().getAccountAssetTransactions("user@test", asset + "#test");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountAssetTransactionsInvalidAssetDomain() {
        for (String host: invalidHosts) {
            ModelQueryBuilder builder = base().getAccountAssetTransactions("user@test", "coin#" + host);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountAssetTransactionsEmptyAsset() {
        ModelQueryBuilder builder = base().getAccountAssetTransactions("user@test", "");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    /* ====================== GetAccountAssets Tests ====================== */

    @Test
    void getAccountAssets() {
        for (String name: validNameSymbols1) {
            UnsignedQuery query = builder.getAccountAssets(name + "@test", name + "#test").build();
            assertTrue(checkProtoQuery(proto(query)));
        }
    }

    @Test
    void getAccountAssetsWithInvalidAccountName() {
        for (String account: invalidNameSymbols1) {
            ModelQueryBuilder builder = base().getAccountAssets(account + "@test", "coin#test");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountAssetsWithInvalidAccountDomain() {
        for (String host: invalidHosts) {
            ModelQueryBuilder builder = base().getAccountAssets("user@" + host, "coin#test");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountAssetsWithEmptyAccount() {
        ModelQueryBuilder builder = base().getAccountAssets("", "coin#test");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void getAccountAssetsWithInvalidAssetName() {
        for (String asset: invalidNameSymbols1) {
            ModelQueryBuilder builder = base().getAccountAssets("user@test", asset + "#test");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountAssetsWithInvalidAssetDomain() {
        for (String host: invalidHosts) {
            ModelQueryBuilder builder = base().getAccountAssets("user@test", "coin#" + host);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountAssetsWithEmptyAsset() {
        ModelQueryBuilder builder = base().getAccountAssets("user@test", "");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    /* ====================== GetRoles Tests ====================== */

    @Test
    void getRoles() {
        UnsignedQuery query = builder.getRoles().build();
        assertTrue(checkProtoQuery(proto(query)));
    }

    /* ====================== GetAssetInfo Tests ====================== */

    @Test
    void getAssetInfo() {
        for (String asset: validNameSymbols1) {
            UnsignedQuery query = builder.getAssetInfo(asset + "#test").build();
            assertTrue(checkProtoQuery(proto(query)));
        }
    }

    @Test
    void getAssetInfoWithInvalidName() {
        for (String asset: invalidNameSymbols1) {
            ModelQueryBuilder builder = base().getAssetInfo(asset + "#test");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAssetInfoWithInvalidDomain() {
        for (String host: invalidHosts) {
            ModelQueryBuilder builder = base().getAssetInfo("coin#" + host);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAssetInfoWithEmptyName() {
        ModelQueryBuilder builder = base().getAssetInfo("");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    /* ====================== GetRolePermissions Tests ====================== */

    @Test
    void getRolePermissions() {
        for (String role: validNameSymbols1) {
            UnsignedQuery query = builder.getRolePermissions(role).build();
            assertTrue(checkProtoQuery(proto(query)));
        }
    }

    @Test
    void getRolePermissionsWithInvalidName() {
        for (String role: invalidNameSymbols1) {
            ModelQueryBuilder builder = base().getRolePermissions(role);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getRolePermissionsWithEmptyName() {
        ModelQueryBuilder builder = base().getRolePermissions("");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    /* ====================== GetTransactions Tests ====================== */

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

    // The following four tests are disabled because there is a need to
    // clarify desired behavior.
    @Disabled
    @Test
    void getTransactionsWithEmptyVector() {
        ModelQueryBuilder builder = base().getTransactions(new HashVector());
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Disabled
    @Test
    void getTransactionsWithInvalidHashSizes() {
        String hashes[] = {
            "",
            "1",
            "1234567890123456789012345678901",
            "123456789012345678901234567890123"
        };

        for (String hash: hashes) {
            HashVector hv = new HashVector();
            hv.add(new Hash(hash));
            ModelQueryBuilder builder = base().getTransactions(hv);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Disabled
    @Test
    void getTransactionsWithOneValidAndOneInvalidHash1() {
        Hash valid = new Hash("12345678901234567890123456789012");
        Hash invalid = new Hash("1");

        HashVector hv = new HashVector();
        hv.add(valid);
        hv.add(invalid);
        ModelQueryBuilder builder = base().getTransactions(hv);
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Disabled
    @Test
    void getTransactionsWithOneValidAndOneInvalidHash2() {
        Hash valid = new Hash("12345678901234567890123456789012");
        Hash invalid = new Hash("1");

        HashVector hv = new HashVector();
        hv.add(invalid);
        hv.add(valid);
        ModelQueryBuilder builder = base().getTransactions(hv);
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    /* ====================== GetAccountDetail Tests ====================== */

    @Test
    void getAccountDetail() {
        for (String account: validNameSymbols1) {
            UnsignedQuery query = builder.getAccountDetail(account + "@test").build();
            assertTrue(checkProtoQuery(proto(query)));
        }
    }

    @Test
    void getAccountDetailWithInvalidName() {
        for (String account: invalidNameSymbols1) {
            ModelQueryBuilder builder = base().getAccountDetail(account + "@test");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountDetailWithInvalidDomain() {
        for (String host: invalidHosts) {
            ModelQueryBuilder builder = base().getAccountDetail("user@" + host);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountDetailWithEmptyName() {
        ModelQueryBuilder builder = base().getAccountDetail("");
        assertThrows(IllegalArgumentException.class, builder::build);
    }
}
