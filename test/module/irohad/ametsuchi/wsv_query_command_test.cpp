/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_wsv_command.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "framework/result_fixture.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"
#include "module/shared_model/builders/protobuf/test_peer_builder.hpp"

namespace iroha {
  namespace ametsuchi {

    using namespace framework::expected;

    class WsvQueryCommandTest : public AmetsuchiTest {
     public:
      void SetUp() override {
        AmetsuchiTest::SetUp();
        sql = std::make_unique<soci::session>(soci::postgresql, pgopt_);

        command = std::make_unique<PostgresWsvCommand>(*sql);
        query = std::make_unique<PostgresWsvQuery>(*sql, factory);

        *sql << init_;
      }

      void TearDown() override {
        sql->close();
        AmetsuchiTest::TearDown();
      }

      std::unique_ptr<soci::session> sql;

      std::unique_ptr<WsvCommand> command;
      std::unique_ptr<WsvQuery> query;
    };

    class RoleTest : public WsvQueryCommandTest {};

    TEST_F(RoleTest, InsertTwoRole) {
      ASSERT_TRUE(val(command->insertRole("role")));
      ASSERT_TRUE(err(command->insertRole("role")));
    }

    class DeletePeerTest : public WsvQueryCommandTest {
     public:
      void SetUp() override {
        WsvQueryCommandTest::SetUp();

        peer = clone(TestPeerBuilder().build());
      }
      std::unique_ptr<shared_model::interface::Peer> peer;
    };

    /**
     * @given storage with peer
     * @when trying to delete existing peer
     * @then peer is successfully deleted
     */
    TEST_F(DeletePeerTest, DeletePeerValidWhenPeerExists) {
      ASSERT_TRUE(val(command->insertPeer(*peer)));

      ASSERT_TRUE(val(command->deletePeer(*peer)));
    }

    // Since mocking database is not currently possible, use SetUp to create
    // invalid database
    class DatabaseInvalidTest : public WsvQueryCommandTest {
      // skip database setup
      void SetUp() override {
        AmetsuchiTest::SetUp();
        sql = std::make_unique<soci::session>(soci::postgresql, pgopt_);

        command = std::make_unique<PostgresWsvCommand>(*sql);
        query = std::make_unique<PostgresWsvQuery>(*sql, factory);
      }
    };

    /**
     * @given not set up database
     * @when performing query to retrieve information from nonexisting tables
     * @then query will return nullopt
     */
    TEST_F(DatabaseInvalidTest, QueryInvalidWhenDatabaseInvalid) {
      EXPECT_FALSE(query->getSignatories("some_account"));
    }
  }  // namespace ametsuchi
}  // namespace iroha
