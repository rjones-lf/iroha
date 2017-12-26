import iroha.protocol.Commands;
import iroha.protocol.BlockOuterClass;
import iroha.protocol.Endpoint;
import iroha.protocol.Queries;

import iroha.protocol.QueryServiceGrpc;
import iroha.protocol.CommandServiceGrpc;
import iroha.protocol.CommandServiceGrpc.CommandServiceBlockingStub;
import iroha.protocol.Endpoint.TxStatus;
import iroha.protocol.Endpoint.TxStatusRequest;
import iroha.protocol.Endpoint.ToriiResponse;

import com.google.protobuf.InvalidProtocolBufferException;
import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import com.google.protobuf.ByteString;

import java.nio.file.Paths;
import java.nio.file.Files;
import java.io.IOException;
import java.math.BigInteger;
import java.lang.Thread;

class TransactionExample {
    static {
        try {
            System.loadLibrary("irohajava");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Native code library failed to load. \n" + e);
            System.exit(1);
        }
    }

    private static ModelCrypto crypto = new ModelCrypto();
    private static ModelTransactionBuilder txBuilder = new ModelTransactionBuilder();
    private static ModelQueryBuilder queryBuilder = new ModelQueryBuilder();
    private static ModelProtoTransaction protoTxHelper = new ModelProtoTransaction();
    private static ModelProtoQuery protoQueryHelper = new ModelProtoQuery();



    public static void main(String[] args) {
        String adminPub = "", adminPriv = "";

        try {
            adminPub = new String(Files.readAllBytes(Paths.get("../admin@test.pub")));
            adminPriv = new String(Files.readAllBytes(Paths.get("../admin@test.priv")));
        } catch (IOException e) {
            System.err.println("Unable to read key files.\n " + e);
            System.exit(1);
        }

        Keypair keys = crypto.convertFromExisting(adminPub, adminPriv);

        long currentTime = System.currentTimeMillis();
        //long startCounter = 1;
        String creator = "admin@test";

        // build transaction (still unsigned)
        UnsignedTx utx = txBuilder.creatorAccountId(creator)
            .createdTime(BigInteger.valueOf(currentTime))
            .createDomain("ru", "user")
            .createAsset("dollar", "ru", (short)2).build();

        // sign transaction and get its binary representation (Blob)
        ByteVector txblob = protoTxHelper.signAndAddSignature(utx, keys).blob();

        // Convert ByteVector to byte array
        byte bs[] = new byte[(int)txblob.size()];
        for (int i = 0; i < txblob.size(); ++i) {
            bs[i] = (byte)txblob.get(i);
        }

        // create proto object
        BlockOuterClass.Transaction protoTx = null;

        try {
            protoTx = BlockOuterClass.Transaction.parseFrom(bs);
        } catch (InvalidProtocolBufferException e) {
            System.err.println("Exception while converting byte array to protobuf:" + e.getMessage());
            System.exit(1);
        }

        // Send transaction to iroha
        ManagedChannel channel = ManagedChannelBuilder.forAddress("localhost", 50051).usePlaintext(true).build();
        CommandServiceBlockingStub stub = CommandServiceGrpc.newBlockingStub(channel);
        stub.torii(protoTx);

        // wait to ensure transaction was processed
        try {
            Thread.sleep(5000);
        }
        catch(InterruptedException ex) {
            Thread.currentThread().interrupt();
        }

        // create status request
        System.out.println("Hash of the transaction: " + utx.hash().hex());

        ByteVector txhash = utx.hash().blob();


        byte bshash[] = new byte[(int)txhash.size()];
        for (int i = 0; i < txhash.size(); ++i) {
            bshash[i] = (byte)txhash.get(i);
        }
        String strHash = new String(bshash);

        TxStatusRequest request = TxStatusRequest.newBuilder().setTxHash(ByteString.copyFrom(bshash)).build();
        ToriiResponse response = stub.status(request);
        String status = response.getTxStatus().name();

        System.out.println("Status of the transaction is: " + status);

        if (!status.equals("COMMITTED")) {
            System.err.println("Your transaction wasn't committed");
            System.exit(1);
        }
    }
}
