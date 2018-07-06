/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.Disabled;

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


    /* Stateless validation rules are listed
     * here https://github.com/hyperledger/iroha/pull/1148
     */

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

    private final String[] validDomains = {
        "test",
        "u9EEA432F",
        "a-hyphen",
        "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad",
        "endWith0",
        "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad." +
        "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad." +
        "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad." +
        "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad"
    };

    private final String[] invalidDomains = {
        "",
        " ",
        "   ",
        "9start.with.digit",
        "-startWithDash",
        "@.is.not.allowed",
        "no space is allowed",
        "endWith-",
        "label.endedWith-.is.not.allowed",
        "aLabelMustNotExceeds63charactersALabelMustNotExceeds63characters",
        "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad." +
        "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad." +
        "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad." +
        "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPadP",
        "257.257.257.257",
        "domain#domain",
        "asd@asd",
        "ab..cd"
    };

    private final String[] invalidKeysBytes = {
        "",
        "a",
        "1111111111111111111111111111111",
        "111111111111111111111111111111111"
    };

    ModelTransactionBuilder base() {
        return new ModelTransactionBuilder()
                .createdTime(BigInteger.valueOf(System.currentTimeMillis()))
                .creatorAccountId("admin@test");
    }

    void setAddPeer() {
        builder.addPeer("123.123.123.123", keys.publicKey());
    }

    Blob proto(UnsignedTx tx) {
        return new ModelProtoTransaction(tx).signAndAddSignature(keys).finish();
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

    /* ====================== AddPeer Tests ====================== */

    @Test
    void addPeer() {
        for (String domain: validDomains) {
            UnsignedTx tx = builder.addPeer(domain + ":123", keys.publicKey()).build();
            assertTrue(checkProtoTx(proto(tx)));
        }
    }

    @Test
    void outdatedAddPeer() {
        setAddPeer();
        for (BigInteger i: new BigInteger[]{BigInteger.valueOf(0),
                BigInteger.valueOf(System.currentTimeMillis() - 100_000_000),
                BigInteger.valueOf(System.currentTimeMillis() + 1000)}) {
            builder.createdTime(i);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void addPeerWithInvalidCreator() {
        setAddPeer();
        for (String account: invalidNameSymbols1) {
            builder.creatorAccountId(account + "@test");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void addPeerWithEmptyCreator() {
        setAddPeer();
        builder.creatorAccountId("");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void addPeerWithInvalidCreatorDomain() {
        setAddPeer();
        for (String domain: invalidDomains) {
            builder.creatorAccountId("admin@" + domain);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void addPeerWithInvalidKeySize() {
        setAddPeer();
        for (String key: invalidKeysBytes) {
            ModelTransactionBuilder builder = base().addPeer("123.123.123.123", new PublicKey(key));
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void addPeerWithInvalidDomain() {
        setAddPeer();
        for (String domain: invalidDomains) {
            ModelTransactionBuilder builder = base().addPeer(domain, keys.publicKey());
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    /* ====================== AddSignatory Tests ====================== */

    @Test
    void addSignatory() {
        for (String accountName: validNameSymbols1) {
            for (String domain: validDomains) {
                String accountId = accountName + "@" + domain;
                UnsignedTx tx = builder.addSignatory(accountId, keys.publicKey()).build();
                assertTrue(checkProtoTx(proto(tx)));
            }
        }
    }

    @Test
    void addSignatoryInvalidAccountName() {
        for (String accountName: invalidNameSymbols1) {
            String accountId = accountName + "@domain";
            ModelTransactionBuilder builder = base().addSignatory(accountId, keys.publicKey());
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void addSignatoryEmptyAccountId() {
        ModelTransactionBuilder builder = base().addSignatory("", keys.publicKey());
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void addSignatoryInvalidDomain() {
        for (String domain: invalidDomains) {
            String accountId = "user@" + domain;
            ModelTransactionBuilder builder = base().addSignatory(accountId, keys.publicKey());
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void addSignatoryInvalidKey() {
        for (String invalidKeyBytes: invalidKeysBytes) {
            ModelTransactionBuilder builder = base().addSignatory("admin@test", new PublicKey(invalidKeyBytes));
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    /* ====================== AddAssetQuantity Tests ====================== */

    @Test
    void addAssetQuantity() {
        UnsignedTx tx = builder.addAssetQuantity("asset#domain", "12.345").build();
        assertTrue(checkProtoTx(proto(tx)));
    }

    @Test
    void addAssetQuantityValidAssets() {
        for (String domain: validDomains) {
            for (String name: validNameSymbols1) {
                UnsignedTx tx = builder.addAssetQuantity(name + "#" + domain, "100").build();
                assertTrue(checkProtoTx(proto(tx)));
            }
        }
    }

    @Test
    void addAssetQuantityInvalidAssetDomain() {
        for (String domain: invalidDomains) {
            ModelTransactionBuilder builder = base().addAssetQuantity("asset#" + domain, "10");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void addAssetZeroQuantity() {
        ModelTransactionBuilder builder = base().addAssetQuantity("asset#domain", "0");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void addAssetQuantityInvalidAssetName() {
        for (String assetName: invalidNameSymbols1) {
            String assetId = assetName + "#test";
            ModelTransactionBuilder builder = base().addAssetQuantity(assetId, "10");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void addAssetQuantityEmptyAsset() {
        ModelTransactionBuilder builder = base().addAssetQuantity("", "10");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void addAssetQuantityInvalidAmount() {
        for (String amount: new String[]{"", "-12", "-13.45", "chars", "chars10"}) {
            ModelTransactionBuilder builder = base().addAssetQuantity("asset#test", amount);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    /* ====================== RemoveSignatory Tests ====================== */

    @Test
    void removeSignatory() {
        for (String accountName: validNameSymbols1) {
            for (String domain: validDomains) {
                String accountId = accountName + "@" + domain;
                UnsignedTx tx = builder.removeSignatory(accountId, keys.publicKey()).build();
                assertTrue(checkProtoTx(proto(tx)));
            }
        }
    }

    @Test
    void removeSignatoryEmptyAccount() {
        ModelTransactionBuilder builder = base().removeSignatory("", keys.publicKey());
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void removeSignatoryInvalidAccount() {
        for (String accountName: invalidNameSymbols1) {
            String accountId = accountName + "@test";
            ModelTransactionBuilder builder = base().removeSignatory(accountId, keys.publicKey());
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void removeSignatoryInvalidAccountDomain() {
        for (String domain: invalidDomains) {
            ModelTransactionBuilder builder = base().removeSignatory("admin@" + domain, keys.publicKey());
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void removeSignatoryInvalidKey() {
        for (String invalidKeyBytes: invalidKeysBytes) {
            ModelTransactionBuilder builder = base().removeSignatory("admin@test", new PublicKey(invalidKeyBytes));
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    /* ====================== CreateAccount Tests ====================== */

    @Test
    void createAccount() {
        for (String accountName: validNameSymbols1) {
            for (String domain: validDomains) {
                UnsignedTx tx = builder.createAccount(accountName, domain, keys.publicKey()).build();
                assertTrue(checkProtoTx(proto(tx)));
            }
        }
    }

    @Test
    void createAccountInvalidAccountName() {
        for (String accountName: invalidNameSymbols1) {
            ModelTransactionBuilder builder = base().createAccount(accountName, "domain", keys.publicKey());
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void createAccountInvalidDomain() {
        for (String domain: invalidDomains) {
            ModelTransactionBuilder builder = base().createAccount("admin", domain, keys.publicKey());
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void createAccountInvalidKey() {
        for (String invalidKeyBytes: invalidKeysBytes) {
            ModelTransactionBuilder builder = base().createAccount("admin", "test", new PublicKey(invalidKeyBytes));
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    /* ====================== CreateDomain Tests ====================== */

    @Test
    void createDomain() {
        for (String role: validNameSymbols1) {
            for (String domain: validDomains) {
                UnsignedTx tx = builder.createDomain(domain, role).build();
                assertTrue(checkProtoTx(proto(tx)));
            }
        }
    }

    @Test
    void createDomainInvalidDomain() {
        for (String domain: invalidDomains) {
            ModelTransactionBuilder builder = base().createDomain(domain, "role");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void createDomainInvalidRole() {
        for (String role: invalidNameSymbols1) {
            ModelTransactionBuilder builder = base().createDomain("domain", role);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    /* ====================== SetAccountQuorum Tests ====================== */


    @Test
    void setAccountQuorum() {
        for (String accountName: validNameSymbols1) {
            for (String domain: validDomains) {
                String accountId = accountName + "@" + domain;
                UnsignedTx tx = builder.setAccountQuorum(accountId, 128).build();
                assertTrue(checkProtoTx(proto(tx)));
            }
        }
    }

    @Test
    void setAccountQuorumInvalidAccount() {
        for (String accountName: invalidNameSymbols1) {
            String accountId = accountName + "@test";
            ModelTransactionBuilder builder = base().setAccountQuorum(accountId, 123);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void setAccountQuorumInvalidDomain() {
        for (String domain: invalidDomains) {
            String accountId = "admin@" + domain;
            ModelTransactionBuilder builder = base().setAccountQuorum(accountId, 123);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void setAccountQuorumEmptyAccount() {
        ModelTransactionBuilder builder = base().setAccountQuorum("", 123);
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void setAccountQuorumInvalidQuantity() {
        for (int quorumSize: new int[]{0, 129, -100}) {
            ModelTransactionBuilder builder = base().setAccountQuorum("admin@test", quorumSize);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    /* ====================== TransferAsset Tests ====================== */

    @Test
    void transferAsset() {
        for (String domain: validDomains) {
            for (int i = 0; i < validNameSymbols1.length; i++) {
                String from = validNameSymbols1[i] + "@" + domain;
                String to = validNameSymbols1[(i + 1) % validNameSymbols1.length] + "@" + domain;
                String asset = validNameSymbols1[(i + 2) % validNameSymbols1.length] + "#" + domain;
                UnsignedTx tx = builder.transferAsset(from, to, asset, "description", "123.456").build();
                assertTrue(checkProtoTx(proto(tx)));
            }
        }
    }

    @Test
    void transferAssetWithValidName() {
        for (String assetName: validNameSymbols1) {
            String assetId = assetName + "#test";
            UnsignedTx tx = builder.transferAsset("from@test", "to@test", assetId, "description", "100").build();
            assertTrue(checkProtoTx(proto(tx)));
        }
    }

    @Test
    void transferAssetWithInvalidName() {
        for (String assetName: invalidNameSymbols1) {
            String assetId = assetName + "#test";
            ModelTransactionBuilder builder = base().transferAsset("from@test", "to@test", assetId, "description", "100");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void transferAssetWithInvalidFromAccount() {
        for (String accountName: invalidNameSymbols1) {
            String accountId = accountName + "@test";
            ModelTransactionBuilder builder = base().transferAsset(accountId, "to@test", "asset#test", "description", "100");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void transferAssetWithInvalidToAccount() {
        for (String accountName: invalidNameSymbols1) {
            String accountId = accountName + "@test";
            ModelTransactionBuilder builder = base().transferAsset("from@test", accountId, "asset#test", "description", "100");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void transferAssetWithEmptyFromAccount() {
        ModelTransactionBuilder builder = base().transferAsset("", "to@test", "asset#test", "description", "100");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void transferAssetWithEmptyToAccount() {
        ModelTransactionBuilder builder = base().transferAsset("from@test", "", "asset#test", "description", "100");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void transferAssetWithInvalidFromDomain() {
        for (String domain: invalidDomains) {
            String accountId = "from@" + domain;
            ModelTransactionBuilder builder = base().transferAsset(accountId, "to@test", "asset#test", "description", "100");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void transferAssetWithInvalidToDomain() {
        for (String domain: invalidDomains) {
            String accountId = "to@" + domain;
            ModelTransactionBuilder builder = base().transferAsset("from@test", accountId, "asset#test", "description", "100");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void transferAssetWithInvalidAssetDomain() {
        for (String domain: invalidDomains) {
            String assetId = "asset#" + domain;
            ModelTransactionBuilder builder = base().transferAsset("from@test", "to@test", assetId, "description", "100");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void transferAssetWithEmptyAssetName() {
        ModelTransactionBuilder builder = base().transferAsset("from@test", "to@test", "", "description", "100");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void transferAssetDescriptionBoundaryValues() {
        for (String description1: new String[]{"", "abcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcde1234"}) {
            UnsignedTx tx = builder.transferAsset("from@test", "to@test", "asset#test", description1, "100").build();
            assertTrue(checkProtoTx(proto(tx)));
        }

        String description2 = "abcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcde12345";
        ModelTransactionBuilder builder = base().transferAsset("from@test", "to@test", "asset#test", description2, "100");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void transferAssetMaximumAmount() {
        BigInteger base = new BigInteger("2");
        BigInteger one = new BigInteger("1");
        BigInteger maxUint256 = base.pow(256).subtract(one);
        BigInteger oversizedInt = maxUint256.add(one);

        String maxAmount1 = maxUint256.toString();
        String maxAmount2 = maxAmount1.substring(0, 10) + "." + maxAmount1.substring(10, maxAmount1.length());

        for (String amount: new String[]{maxAmount1, maxAmount2}) {
            UnsignedTx tx = builder.transferAsset("from@test", "to@test", "asset#test", "description", amount).build();
            assertTrue(checkProtoTx(proto(tx)));
        }

        ModelTransactionBuilder mtb = base().transferAsset("from@test", "to@test", "asset#test", "description", oversizedInt.toString());
        assertThrows(IllegalArgumentException.class, mtb::build);
    }


    /* ====================== SetAccountDetail Tests ====================== */

    @Test
    void setAccountDetail() {
        for (String accountName: validNameSymbols1) {
            for (String domain: validDomains) {
                String accountId = accountName + "@" + domain;
                UnsignedTx tx = builder.setAccountDetail(accountId, "fyodor", "kek").build();
                assertTrue(checkProtoTx(proto(tx)));
            }
        }
    }

    @Test
    void setAccountDetailInvalidAccount() {
        for (String accountName: invalidNameSymbols1) {
            String accountId = accountName + "@test";
            ModelTransactionBuilder builder = base().setAccountDetail(accountId, "fyodor", "true");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void setAccountDetailEmptyAccount() {
        ModelTransactionBuilder builder = base().setAccountDetail("", "fyodor", "true");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void setAccountDetailInvalidAccountDomain() {
        for (String domain: invalidDomains) {
            String accountId = "admin@" + domain;
            ModelTransactionBuilder builder = base().setAccountDetail(accountId, "fyodor", "true");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void setAccountDetailValidKey() {
        for (String key: validNameSymbols2) {
            UnsignedTx tx = builder.setAccountDetail("admin@test", key, "true").build();
            assertTrue(checkProtoTx(proto(tx)));
        }
    }

    @Test
    void setAccountDetailInvalidKey() {
        for (String key: invalidNameSymbols2) {
            ModelTransactionBuilder builder = base().setAccountDetail("admin@test", key, "true");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void setAccountDetailValidValue() {
        int length = 4 * 1024 * 1024;
        StringBuilder sb = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            sb.append("a");
        }

        for (String value: new String[]{"", sb.toString()}) {
            UnsignedTx tx = builder.setAccountDetail("admin@test", "fyodor", value).build();
            assertTrue(checkProtoTx(proto(tx)));
        }
    }

    @Test
    void setAccountDetailWithOversizedValue() {
        int length = 4 * 1024 * 1024 + 1;
        StringBuilder sb = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            sb.append("a");
        }

        ModelTransactionBuilder builder = base().setAccountDetail("admin@test", "fyodor", sb.toString());
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    /* ====================== AppendRole Tests ====================== */

    @Test
    void appendRole() {
        for (String account: validNameSymbols1) {
            for (String domain: validDomains) {
                String accountId = account + "@" + domain;
                UnsignedTx tx = builder.appendRole(accountId, account).build();
                assertTrue(checkProtoTx(proto(tx)));
            }
        }
    }

    @Test
    void appendRoleInvalidAccount() {
        for (String account: invalidNameSymbols1) {
            String accountId = account + "@test";
            ModelTransactionBuilder builder = base().appendRole(accountId, "user");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void appendRoleInvalidDomain() {
        for (String domain: invalidDomains) {
            String accountId = "admin@" + domain;
            ModelTransactionBuilder builder = base().appendRole(accountId, "user");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void appendRoleWithInvalidName() {
        for (String role: invalidNameSymbols1) {
            ModelTransactionBuilder builder = base().appendRole("admin@test", role);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }


    /* ====================== CreateAsset Tests ====================== */

    @Test
    void createAsset() {
        for (String assetName: validNameSymbols1) {
            for (String domain: validDomains) {
                UnsignedTx tx = builder.createAsset(assetName, domain, (short) 6).build();
                assertTrue(checkProtoTx(proto(tx)));
            }
        }
    }

    @Test
    void createAssetWithInvalidName() {
        for (String asset: invalidNameSymbols1) {
            ModelTransactionBuilder builder = base().createAsset(asset, "test", (short) 6);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void createAssetWithInvalidDomain() {
        for (String domain: invalidDomains) {
            ModelTransactionBuilder builder = base().createAsset("asset", domain, (short) 6);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void createAssetWithZeroPrecision() {
        UnsignedTx tx = builder.createAsset("asset", "test", (short) 0).build();
        assertTrue(checkProtoTx(proto(tx)));
    }

    /* ====================== CreateRole Tests ====================== */

    @Test
    void createRole() {
        RolePermissionSet permissions = new RolePermissionSet();
        permissions.set(Role.kReceive);
        permissions.set(Role.kGetRoles);
        for (String role: validNameSymbols1) {
            UnsignedTx tx = builder.createRole(role, permissions).build();
            assertTrue(checkProtoTx(proto(tx)));
        }
    }

    @Test
    void createRoleWithInvalidName() {
        RolePermissionSet permissions = new RolePermissionSet();
        permissions.set(Role.kReceive);
        permissions.set(Role.kGetRoles);
        for (String role: invalidNameSymbols1) {
            ModelTransactionBuilder mtb = base().createRole(role, permissions);
            assertThrows(IllegalArgumentException.class, mtb::build);
        }
    }

    @Test
    void createRoleEmptyPermissions() {
        UnsignedTx tx = builder.createRole("new_role", new RolePermissionSet()).build();
        assertTrue(checkProtoTx(proto(tx)));
    }

    /* ====================== DetachRole Tests ====================== */

    @Test
    void detachRole() {
        for (String name: validNameSymbols1) {
            for (String domain: validDomains) {
                String accountId = name + "@" + domain;
                UnsignedTx tx = builder.detachRole(accountId, name).build();
                assertTrue(checkProtoTx(proto(tx)));
            }
        }
    }

    @Test
    void detachRoleInvalidAccountName() {
        for (String accountName: invalidNameSymbols1) {
            String accountId = accountName + "@test";
            ModelTransactionBuilder builder = base().detachRole(accountId, "role");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void detachRoleInvalidDomain() {
        for (String domain: invalidDomains) {
            String accountId = "admin@" + domain;
            ModelTransactionBuilder builder = base().detachRole(accountId, "role");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void detachRoleWithEmptyAccount() {
        ModelTransactionBuilder builder = base().detachRole("", "role");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void detachRoleWithInvalidName() {
        for (String role: invalidNameSymbols1) {
            ModelTransactionBuilder builder = base().detachRole("admin@test", role);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    /* ====================== GrantPermission Tests ====================== */

    @Test
    void grantPermission() {
        for (String accountName: validNameSymbols1) {
            for (String domain: validDomains) {
                String accountId = accountName + "@" + domain;
                UnsignedTx tx = builder.grantPermission(accountId, Grantable.kSetMyQuorum).build();
                assertTrue(checkProtoTx(proto(tx)));
            }
        }
    }

    @Test
    void grantPermissionInvalidAccount() {
        for (String accountName: invalidNameSymbols1) {
            for (String domain: validDomains) {
                String accountId = accountName + "@" + domain;
                ModelTransactionBuilder builder = base().grantPermission(accountId, Grantable.kSetMyQuorum);
                assertThrows(IllegalArgumentException.class, builder::build);
            }
        }
    }

    @Test
    void grantPermissionEmptyAccount() {
        ModelTransactionBuilder builder = base().grantPermission("", Grantable.kSetMyQuorum);
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    /* ====================== RevokePermission Tests ====================== */

    @Test
    void revokePermission() {
        for (String accountName: validNameSymbols1) {
            for (String domain: validDomains) {
                UnsignedTx tx = builder.revokePermission(accountName + "@" + domain, Grantable.kSetMyQuorum).build();
                assertTrue(checkProtoTx(proto(tx)));
            }
        }
    }

    @Test
    void revokePermissionInvalidAccount() {
        for (String accountName: invalidNameSymbols1) {
            ModelTransactionBuilder builder = base().revokePermission(accountName + "@test", Grantable.kSetMyQuorum);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void revokePermissionInvalidDomain() {
        for (String domain: invalidDomains) {
            ModelTransactionBuilder builder = base().revokePermission("admin@" + domain, Grantable.kSetMyQuorum);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void revokePermissionEmptyAccount() {
        ModelTransactionBuilder builder = base().revokePermission("", Grantable.kSetMyQuorum);
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    /* ====================== SubtractAssetQuantity Tests ====================== */

    @Test
    void subtractAssetQuantity() {
        for (String name: validNameSymbols1) {
            for (String domain: validDomains) {
                UnsignedTx tx = builder.subtractAssetQuantity(name + "#" + domain, "10.22").build();
                assertTrue(checkProtoTx(proto(tx)));
            }
        }
    }

    @Test
    void subtractAssetQuantityInvalidAsset() {
        for (String asset: invalidNameSymbols1) {
            ModelTransactionBuilder builder = base().subtractAssetQuantity(asset + "#test", "10");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void subtractAssetQuantityInvalidAssetDomain() {
        for (String domain: invalidDomains) {
            ModelTransactionBuilder builder = base().subtractAssetQuantity("coin#" + domain, "10");
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void subtractAssetQuantityEmptyAsset() {
        ModelTransactionBuilder builder = base().subtractAssetQuantity("", "10");
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void subtractAssetQuantityInvalidAmount() {
        String invalidAmounts[] = {
            "",
            "0",
            "chars",
            "-10",
            "10chars",
            "10.10.10"
        };

        for (String amount: invalidAmounts) {
            ModelTransactionBuilder builder = base().subtractAssetQuantity("coin#test", amount);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }
}
