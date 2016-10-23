#include "../../consensus/connection/connection.hpp"

#include "../../util/logger.hpp"


#include <cstdint>
#include <cstdio>
#include <signal.h>
#include <util/CommandOptionParser.h>
#include <thread>
#include <Aeron.h>
#include <array>
#include <iostream>

#include <cstdint>
#include <cstdio>
#include <signal.h>
#include <util/CommandOptionParser.h>
#include <thread>
#include <Aeron.h>
#include <array>
#include <iostream>
#include <atomic>

using namespace aeron::util;
using namespace aeron;

std::atomic<int> counter (0);

static const std::chrono::duration<long, std::milli> IDLE_SLEEP_MS(1);
static const int FRAGMENTS_LIMIT = 10;

namespace connection {

    std::int32_t streamId = 10;

    std::thread subscription_thread;
    std::map<std::string, std::shared_ptr<Publication>> publications;
    std::vector<std::function<void(std::string from,std::string message)>> receivers;
    std::shared_ptr<Aeron> aeron;
    typedef std::array<std::uint8_t, 1024> buffer_t;

    bool subscription_running(true);

    fragment_handler_t receiveMessage() {
        return [&](AtomicBuffer& buffer, util::index_t offset, util::index_t length, Header& header)
        {
            std::string raw_data = std::string((char *)buffer.buffer() + offset, (unsigned long)length);
            // WIP parse json.
            for(auto& f : receivers) {
                f("From", raw_data);
            }
        };
    }

    int initialize_peer(std::unordered_map<std::string,std::string> data){
        aeron::Context context;

        context.newSubscriptionHandler(
            [](const std::string& channel, std::int32_t streamId, std::int64_t correlationId){});
        context.newPublicationHandler(
            [](const std::string& channel, std::int32_t streamId, std::int32_t sessionId, std::int64_t correlationId){});

        aeron = Aeron::connect(context);
        return 0;
    }

    int exec_subscription(std::string ip){
        try{
            logger::info("connection", "subscript ["+ ip +"]");

            std::int64_t sid = aeron->addSubscription("aeron:udp?endpoint="+ip+":40123", streamId);
            auto subscription = aeron->findSubscription(sid);
            
            while (!subscription) {
                std::this_thread::yield();
                subscription = aeron->findSubscription(sid);
            }

            fragment_handler_t handler = receiveMessage();
            subscription_thread = std::thread([subscription,handler](){
                while (subscription_running){
                    const int fragmentsRead = subscription->poll(handler, FRAGMENTS_LIMIT);
                }
                logger::info("connection", "subscription halt");
            });
            logger::info("connection", "subscription begin run");
        }catch (SourcedException& e) {
            logger::error("connection", "FAILED: "+ std::string(e.what()) +" : "+ std::string(e.where()));
            return -1;
        }catch (std::exception& e) {
            logger::error("connection", "FAILED: "+ std::string(e.what()));
            return -1;
        }
    }

    void addPublication(std::string ip){
        std::int64_t pid = aeron->addPublication( "aeron:udp?endpoint="+ip+":40123", streamId);
        auto publication =  aeron->findPublication(pid);
        while (!publication) {
            std::this_thread::yield();
            publication = aeron->findPublication(pid);
        }
        logger::info("connection", "publication ["+ ip +"]");
        publications.insert(std::pair<std::string, std::shared_ptr<Publication>>{ ip, publication});
    }

    bool send(const std::string& to,const std::string& msg) {
        logger::info("connection", "Start send()");
        if(publications.find(to) == publications.end()){
            logger::error("connection", to + " is not registerd");
            return false;
        }
        try{
            char* message = const_cast<char*>(msg.c_str());
            AERON_DECL_ALIGNED(std::uint8_t buffer[512], 16);        
            AtomicBuffer srcBuffer(&buffer[0], 512);
            srcBuffer.putBytes(0, reinterpret_cast<std::uint8_t *>(message), strlen(message));
            const std::int64_t result = publications[to]->offer(srcBuffer, 0, strlen(message));
            if (result < 0){
                if (NOT_CONNECTED == result){
                    logger::error("connection", " not connected yet.");
                }else if (BACK_PRESSURED == result){
                    logger::error("connection", " back pressured.");
                }else{
                    logger::error("connection", "unknown");
                }
            }else{
                logger::debug("connection", "Ok");
            }
            if (!publications[to]->isConnected()){
                logger::error("connection", "No active subscribers detected");
            }
            return true;
        }catch (SourcedException& e) {
            logger::error("connection", "FAILED: "+ std::string(e.what()) +" : "+ std::string(e.where()));
            return false;
        }catch (std::exception& e) {
            logger::error("connection", "FAILED: "+ std::string(e.what()));
            return false;
        }
    }

    bool sendAll(const std::string& msg) {
        for(auto& p : publications){
            send( p.first, msg);
        }
        return false;
    }

    bool receive(std::function<void(std::string from,std::string message)> callback) {
        receivers.push_back(callback);
        return true;
    }
};