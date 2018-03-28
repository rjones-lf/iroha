/**
 * Copyright 2018 HUAWEI. All Rights Reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

'use strict'

function blob2array (blob) {
  var bytearray = new Uint8Array(blob.size())
  for (let i = 0; i < blob.size(); ++i) {
    bytearray[i] = blob.get(i)
  }
  return bytearray
}

var iroha = require('../index')
var txBuilder = new iroha.ModelTransactionBuilder()
var queryBuilder = new iroha.ModelQueryBuilder()
var crypto = new iroha.ModelCrypto()
var protoTxHelper = new iroha.ModelProtoTransaction()
var protoQueryHelper = new iroha.ModelProtoQuery()
var fs = require('fs')
var adminPriv = fs.readFileSync('admin@test.priv').toString()
var adminPub = fs.readFileSync('admin@test.pub').toString()

var keys = crypto.convertFromExisting(adminPub, adminPriv)

var currentTime = Date.now()
var startTxCounter = 1
var startQueryCounter = 1
var creator = 'admin@test'

// build transaction
var tx = txBuilder
  .creatorAccountId(creator)
  .txCounter(startTxCounter)
  .createdTime(currentTime)
  .createDomain('ru', 'user')
  .createAsset('dollar', 'ru', 2)
  .build()

// sign transaction and get its binary representation (Blob)
var txblob = protoTxHelper.signAndAddSignature(tx, keys).blob()
var txArray = blob2array(txblob)
// create proto object and send to iroha
var blockTransaction = require('../pb/block_pb.js').Transaction // block_pb2.Transaction()
var protoTx = blockTransaction.deserializeBinary(txArray)
console.log(protoTx.getPayload().getCreatorAccountId())

var grpc = require('grpc')
var endpointGrpc = require('../pb/endpoint_grpc_pb.js')
var client = new endpointGrpc.CommandServiceClient(
  'localhost:50051',
  grpc.credentials.createInsecure()
)
var txHashBlob = tx.hash().blob()
var txHash = blob2array(txHashBlob)
var p = new Promise((resolve, reject) => {
  console.log('Submit transaction...')
  client.torii(protoTx, (err, data) => {
    if (err) {
      reject(err)
    } else {
      console.log('Submitted transaction successfully')
      resolve()
    }
  })
})

p
  .then(() => {
    console.log('Sleep 5 seconds...')
    return sleep(5000)
  })
  .then(() => {
    console.log('Send transaction status request...')
    return new Promise((resolve, reject) => {
      // create status request
      var endpointPb = require('../pb/endpoint_pb.js')
      var request = new endpointPb.TxStatusRequest()
      request.setTxHash(txHash)
      client.status(request, (err, response) => {
        if (err) {
          reject(err)
        } else {
          let status = response.getTxStatus()
          let TxStatus = require('../pb/endpoint_pb.js').TxStatus
          let statusName = getProtoEnumName(
            TxStatus,
            'iroha.protocol.TxStatus',
            status
          )
          console.log('Got transaction status: ' + statusName)
          if (statusName !== 'COMMITTED') {
            reject(new Error("Your transaction wasn't committed"))
          } else {
            resolve()
          }
        }
      })
    })
  })
  .then(() => {
    console.log('Query transaction...')
    let query = queryBuilder
      .creatorAccountId(creator)
      .createdTime(Date.now())
      .queryCounter(startQueryCounter)
      .getAssetInfo('dollar#ru')
      .build()
    let queryBlob = protoQueryHelper.signAndAddSignature(query, keys).blob()
    let pbQuery = require('../pb/queries_pb.js').Query
    let queryArray = blob2array(queryBlob)
    let protoQuery = pbQuery.deserializeBinary(queryArray)
    let client = new endpointGrpc.QueryServiceClient(
      'localhost:50051',
      grpc.credentials.createInsecure()
    )
    return new Promise((resolve, reject) => {
      client.find(protoQuery, (err, response) => {
        if (err) {
          reject(err)
        } else {
          console.log('Submitted transaction successfully')
          let type = response.getResponseCase()
          let responsePb = require('../pb/responses_pb.js')
          let name = getProtoEnumName(
            responsePb.QueryResponse.ResponseCase,
            'iroha.protocol.QueryResponse',
            type
          )
          if (name !== 'ASSET_RESPONSE') {
            reject(new Error('Query response error'))
          } else {
            let asset = response.getAssetResponse().getAsset()
            console.log(
              'Asset Id = ' +
                asset.getAssetId() +
                ' , Precision = ' +
                asset.getPrecision()
            )
            resolve()
          }
        }
      })
    })
  })
  .then(() => {
    console.log('done!')
  })
  .catch(err => {
    console.log(err)
  })

function sleep (ms) {
  return new Promise(resolve => setTimeout(resolve, ms))
}

var protoEnumName = {}
function getProtoEnumName (obj, key, value) {
  if (protoEnumName.hasOwnProperty(key)) {
    if (protoEnumName[key].length < value) {
      return 'unknown'
    } else {
      return protoEnumName[key][value]
    }
  } else {
    protoEnumName[key] = []
    for (var k in obj) {
      let idx = obj[k]
      if (isNaN(idx)) {
        console.log(
          'getProtoEnumName:wrong enum value, now is type of ' +
            typeof idx +
            ' should be integer'
        )
      } else {
        protoEnumName[key][idx] = k
      }
    }
    return getProtoEnumName(obj, key, value)
  }
}
