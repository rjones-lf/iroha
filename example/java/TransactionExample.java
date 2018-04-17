import iroha.protocol.Commands;
import iroha.protocol.BlockOuterClass;
import iroha.protocol.Endpoint;
import iroha.protocol.Queries;
import iroha.protocol.Queries.Query;
import iroha.protocol.Queries.GetAssetInfo;

import iroha.protocol.QueryServiceGrpc;
import iroha.protocol.QueryServiceGrpc.QueryServiceBlockingStub;
import iroha.protocol.CommandServiceGrpc;
import iroha.protocol.CommandServiceGrpc.CommandServiceBlockingStub;
import iroha.protocol.Endpoint.TxStatus;
import iroha.protocol.Endpoint.TxStatusRequest;
import iroha.protocol.Endpoint.ToriiResponse;
import iroha.protocol.Responses.QueryResponse;
import iroha.protocol.Responses.AssetResponse;
import iroha.protocol.Responses.Asset;

import com.google.protobuf.InvalidProtocolBufferException;
import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import com.google.protobuf.ByteString;
import com.google.protobuf.Descriptors.FieldDescriptor;

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

    public static byte[] toByteArray(ByteVector blob) {
        byte bs[] = new byte[(int)blob.size()];
        for (int i = 0; i < blob.size(); ++i) {
            bs[i] = (byte)blob.get(i);
        }
        return bs;
    }

    public static String readKeyFromFile(String path) {
        try {
            return new String(Files.readAllBytes(Paths.get(path)));
        } catch (IOException e) {
            System.err.println("Unable to read key files.\n " + e);
            System.exit(1);
        }
        return "";
    }

    public static void main(String[] args) {
        Keypair keys = crypto.convertFromExisting(readKeyFromFile("../admin@test.pub"),
            readKeyFromFile("../admin@test.priv"));

        long currentTime = System.currentTimeMillis();
        String creator = "admin@test";

        long startQueryCounter = 1;

        // build transaction (still unsigned)
        UnsignedTx utx = txBuilder.creatorAccountId(creator)
            .createdTime(BigInteger.valueOf(currentTime))
            .createDomain("ru", "user")
            .createAsset("dollar", "ru", (short)2).build();

        // sign transaction and get its binary representation (Blob)
        ByteVector txblob = protoTxHelper.signAndAddSignature(utx, keys).blob();

        // Convert ByteVector to byte array
        byte bs[] = toByteArray(txblob);

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
        byte bshash[] = toByteArray(txhash);

        TxStatusRequest request = TxStatusRequest.newBuilder().setTxHash(ByteString.copyFrom(bshash)).build();
        ToriiResponse response = stub.status(request);
        String status = response.getTxStatus().name();

        System.out.println("Status of the transaction is: " + status);

        if (!status.equals("COMMITTED")) {
            System.err.println("Your transaction wasn't committed");
            System.exit(1);
        }

        // query result of transaction we've just sent
        UnsignedQuery uquery = queryBuilder.creatorAccountId(creator)
            .queryCounter(BigInteger.valueOf(startQueryCounter))
            .createdTime(BigInteger.valueOf(currentTime))
            .getAssetInfo("dollar#ru")
            .build();
        ByteVector queryBlob = protoQueryHelper.signAndAddSignature(uquery, keys).blob();
        byte bquery[] = toByteArray(queryBlob);

        Query protoQuery = null;
        try {
            protoQuery = Query.parseFrom(bquery);
        } catch (InvalidProtocolBufferException e) {
            System.err.println("Exception while converting byte array to protobuf:" + e.getMessage());
            System.exit(1);
        }

        QueryServiceBlockingStub queryStub = QueryServiceGrpc.newBlockingStub(channel);
        QueryResponse queryResponse = queryStub.find(protoQuery);

        FieldDescriptor fieldDescriptor = queryResponse.getDescriptorForType().findFieldByName("asset_response");
        if (!queryResponse.hasField(fieldDescriptor)) {
            System.err.println("Query response error");
            System.exit(1);
        } else {
            System.out.println("Query responsed with asset response");
        }

        Asset asset = queryResponse.getAssetResponse().getAsset();
        System.out.println("Asset Id = " + asset.getAssetId());
        System.out.println("Precision = " + asset.getPrecision());
        System.out.println("done!");
    }
}
