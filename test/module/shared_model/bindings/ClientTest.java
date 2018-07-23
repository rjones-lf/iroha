/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.Disabled;

import java.math.BigInteger;
import java.io.InputStream;
import java.io.IOException;

import iroha.protocol.*;
import iroha.protocol.TransactionOuterClass.Transaction;
import iroha.protocol.Commands.Command;
import com.google.protobuf.ByteString;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

public class ClientTest {
  static {
    try {
      System.loadLibrary("irohajava");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. \n" + e);
      System.exit(1);
    }
  }

  private Keypair keys;

  @BeforeEach
  void setUp() {
    keys = new ModelCrypto().generateKeypair();
  }

  Command.Builder validAddPeer() {
    return Command.newBuilder().setAddPeer(Commands.AddPeer.newBuilder().setPeer(
        Primitive.Peer.newBuilder()
            .setAddress("127.0.0.1:50500")
            .setPeerKey(ByteString.copyFrom(String.format("%32.32s", " ").getBytes()))));
  }

  Transaction.Payload.ReducedPayload.Builder defaultPayload() {
    return Transaction.Payload.ReducedPayload.newBuilder()
        .setCreatorAccountId("admin@test")
        .setCreatedTime(System.currentTimeMillis())
        .setQuorum(1);
  }

  ByteVector serialize(Transaction tx) {
    ByteVector vec = new ByteVector();
    for (byte b : tx.toByteArray()) {
      vec.add(b);
    }
    return vec;
  }

  class ParseInputStream extends InputStream {
    ParseInputStream(ByteVector vec) {
      this.vec = vec;
      index = 0;
    }

    @Override
    public int read() {
      if (vec.size() > index) {
        return vec.get(index++);
      }
      return -1;
    }

    ByteVector vec;
    int index;
  }

  Transaction deserialize(ByteVector vec) {
    System.out.println(vec.size());
    try {
      return Transaction.parseFrom(new ParseInputStream(vec));
    } catch (IOException e) {
      System.out.println(e.getMessage());
      System.exit(3);
    }
    return null;
  }

  @Test
  void hash() {
    Transaction tx = Transaction.newBuilder()
                         .setPayload(Transaction.Payload.newBuilder().setReducedPayload(
                             defaultPayload().addCommands(validAddPeer())))
                         .build();
    ByteVector h = iroha.hashTransaction(serialize(tx));
    assertEquals(h.size(), 32);
  }

  @Test
  void sign() {
    Transaction tx = Transaction.newBuilder()
                         .setPayload(Transaction.Payload.newBuilder().setReducedPayload(
                             defaultPayload().addCommands(validAddPeer())))
                         .build();
    assertEquals(tx.getSignaturesList().size(), 0);
    Transaction signed_tx = deserialize(iroha.signTransaction(serialize(tx), keys));
    assertEquals(signed_tx.getSignaturesList().size(), 1);
  }

  @Test
  void validateWithoutCommand() {
    Transaction tx = deserialize(iroha.signTransaction(
        serialize(
            Transaction.newBuilder()
                .setPayload(Transaction.Payload.newBuilder().setReducedPayload(defaultPayload()))
                .build()),
        keys));
    assertThrows(IllegalArgumentException.class, () -> iroha.validateTransaction(serialize(tx)));
  }

  @Test
  void validateUnsigned() {
    Transaction tx = Transaction.newBuilder()
                         .setPayload(Transaction.Payload.newBuilder().setReducedPayload(
                             defaultPayload().addCommands(validAddPeer())))
                         .build();
    assertThrows(IllegalArgumentException.class, () -> iroha.validateTransaction(serialize(tx)));
  }

  @Test
  void validateCorrect() {
    Transaction tx = deserialize(
        iroha.signTransaction(serialize(Transaction.newBuilder()
                                 .setPayload(Transaction.Payload.newBuilder().setReducedPayload(
                                     defaultPayload().addCommands(validAddPeer())))
                                 .build()),
            keys));
    iroha.validateTransaction(serialize(tx));
  }
}
