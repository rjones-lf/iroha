/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

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

public class BlocksQueryTest {
    static {
        try {
            System.loadLibrary("irohajava");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Native code library failed to load. \n" + e);
            System.exit(1);
        }
    }

    private Keypair keys;
    private ModelBlocksQueryBuilder builder;

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

    ModelBlocksQueryBuilder base() {
        return new ModelBlocksQueryBuilder().queryCounter(BigInteger.valueOf(123))
                .createdTime(BigInteger.valueOf(System.currentTimeMillis()))
                .creatorAccountId("admin@test");
    }

    Blob proto(UnsignedBlockQuery query) {
        return new ModelProtoBlocksQuery(query).signAndAddSignature(keys).finish();
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

    @BeforeEach
    void setUp() {
        keys = new ModelCrypto().generateKeypair();
        builder = base();
    }


    /* ====================== BlocksQuery Tests ====================== */


    @Test
    void getAccountWithInvalidCreator() {
        for (String accountName: invalidNameSymbols1) {
            for (String domain: validDomains) {
                ModelBlocksQueryBuilder builder = base().creatorAccountId(accountName + "@" + domain);
                assertThrows(IllegalArgumentException.class, builder::build);
            }
        }
    }

    @Test
    void getAccountWithValidCreator() {
        for (String accountName: validNameSymbols1) {
            for (String domain: validDomains) {
                ModelBlocksQueryBuilder builder = base().creatorAccountId(accountName + "@" + domain);
                UnsignedBlockQuery query = builder.build();
                assertTrue(checkProtoQuery(proto(query)));
            }
        }
    }

    @Test
    void getAccountWithInvalidCreatorDomain() {
        for (String domain: invalidDomains) {
            ModelBlocksQueryBuilder builder = base().creatorAccountId("user@" + domain);
            assertThrows(IllegalArgumentException.class, builder::build);
        }
    }

    @Test
    void getAccountWithInvalidTimestamp() {
        ModelBlocksQueryBuilder builder = base().createdTime(BigInteger.valueOf(100000));
        assertThrows(IllegalArgumentException.class, builder::build);
    }

    @Test
    void getAccountWithValidTimestamp() {
        ModelBlocksQueryBuilder builder = base().createdTime(BigInteger.valueOf(System.currentTimeMillis()));
        UnsignedBlockQuery query = builder.build();
        assertTrue(checkProtoQuery(proto(query)));
    }
}
