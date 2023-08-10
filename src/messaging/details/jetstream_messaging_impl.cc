/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/


#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <utility>
#include <memory>
#include <algorithm>
#include <ranges>
#include <cassert>
#include <climits>
#include <atomic>
#include <cmath>
#include <array>
#include <charconv>
#include <cassert>
#include <regex>
#include <boost/uuid/uuid.hpp>            
#include <boost/uuid/uuid_generators.hpp> 
#include <boost/uuid/uuid_io.hpp>

#include <sys/types.h>
#include <unistd.h>

#include <nats/nats.h>

#include "./jetstream_messaging_impl.h"

/* TODO raise the limits */
constexpr int max_msgs_in_flight = 10240;


static const char * _jsError_GetText(jsErrCode errCode) {

    /* Add JS error codes that may not be defined in the c client */
	#ifndef JSStreamOfflineErr
		#define JSStreamOfflineErr 10118
	#endif    
	#ifndef JSConsumerOfflineErr
		#define JSConsumerOfflineErr 10119
	#endif    
	#ifndef JSNoLimitsErr
		#define JSNoLimitsErr 10120
	#endif    
	#ifndef JSConsumerMaxPendingAckExcessErrF
		#define JSConsumerMaxPendingAckExcessErrF 10121
	#endif
	#ifndef JSStreamMaxStreamBytesExceeded
		#define JSStreamMaxStreamBytesExceeded 10122
	#endif 
	#ifndef JSStreamMoveAndScaleErr
		#define JSStreamMoveAndScaleErr 10123
	#endif    
	#ifndef JSStreamMoveInProgressF
		#define JSStreamMoveInProgressF 10124
	#endif    
	#ifndef JSConsumerMaxRequestBatchExceededF
		#define JSConsumerMaxRequestBatchExceededF 10125
	#endif    
	#ifndef JSConsumerReplicasExceedsStream
		#define JSConsumerReplicasExceedsStream 10126
	#endif    
	#ifndef JSConsumerNameContainsPathSeparatorsErr
		#define JSConsumerNameContainsPathSeparatorsErr 10127
	#endif    
	#ifndef JSStreamNameContainsPathSeparatorsErr
		#define JSStreamNameContainsPathSeparatorsErr 10128
	#endif    
	#ifndef JSStreamMoveNotInProgress
		#define JSStreamMoveNotInProgress 10129
	#endif    
	#ifndef JSStreamNameExistRestoreFailedErr
		#define JSStreamNameExistRestoreFailedErr 10130
	#endif    
	#ifndef JSConsumerCreateFilterSubjectMismatchErr
		#define JSConsumerCreateFilterSubjectMismatchErr 10131
	#endif    
	#ifndef JSConsumerCreateDurableAndNameMismatch
		#define JSConsumerCreateDurableAndNameMismatch 10132
	#endif    
	#ifndef JSReplicasCountCannotBeNegative
		#define JSReplicasCountCannotBeNegative 10133
	#endif    

    /* Turn off warnings about switch statement */
    #pragma clang diagnostic push
    #pragma GCC diagnostic push
    #pragma clang diagnostic ignored "-Wswitch"
    #pragma GCC diagnostic ignored "-Wswitch"

    /* Return an error string based on error code */
    switch(errCode) {

        case 0:
            return "No JS error";

        case JSAccountResourcesExceededErr: 	
            return "Resource limits exceeded for account.";

        case JSBadRequestErr: 	
            return "Bad request.";

        case JSClusterIncompleteErr: 	
            return "Incomplete results.";

        case JSClusterNoPeersErr: 	
            return "No suitable peers for placement.";

        case JSClusterNotActiveErr:
            return "JetStream not in clustered mode.";

        case JSClusterNotAssignedErr:
            return "JetStream cluster not assigned to this server.";

        case JSClusterNotAvailErr:
            return "JetStream system temporarily unavailable.";
        
        case JSClusterNotLeaderErr:
            return "JetStream cluster can not handle request.";
        
        case JSClusterRequiredErr:
            return "JetStream clustering support required.";
        
        case JSClusterTagsErr:
            return "Tags placement not supported for operation.";
        
        case JSConsumerCreateErr:
            return "General consumer creation failure string.";
        
        case JSConsumerNameExistErr:
            return "Consumer name already in use.";
        
        case JSConsumerNotFoundErr:
            return "Consumer not found.";
        case JSSnapshotDeliverSubjectInvalidErr:
            return "Deliver subject not valid.";
        
        case JSConsumerDurableNameNotInSubjectErr:
            return "Consumer expected to be durable but no durable name set in subject.";
        
        case JSConsumerDurableNameNotMatchSubjectErr:
            return "Consumer name in subject does not match durable name in request.";
        
        case JSConsumerDurableNameNotSetErr:
            return "Consumer expected to be durable but a durable name was not set.";
        
        case JSConsumerEphemeralWithDurableInSubjectErr:
            return "Consumer expected to be ephemeral but detected a durable name set in subject.";
        
        case JSConsumerEphemeralWithDurableNameErr:
            return "Consumer expected to be ephemeral but a durable name was set in request.";
        
        case JSStreamExternalApiOverlapErr:
            return "Stream external api prefix must not overlap.";
        
        case JSStreamExternalDelPrefixOverlapsErr:
            return "Stream external delivery prefix overlaps with stream subject.";
        
        case JSInsufficientResourcesErr:
            return "Insufficient resources.";
        
        case JSStreamInvalidExternalDeliverySubjErr:
            return "Stream external delivery prefix must not contain wildcards.";
        
        case JSInvalidJSONErr:
            return "Invalid JSON.";
        
        case JSMaximumConsumersLimitErr:
            return "Maximum consumers exceeds account limit.";
        
        case JSMaximumStreamsLimitErr:
            return "Maximum number of streams reached.";
        
        case JSMemoryResourcesExceededErr:
            return "Insufficient memory resources available.";
        
        case JSMirrorConsumerSetupFailedErr:
            return "Generic mirror consumer setup failure.";
        
        case JSMirrorMaxMessageSizeTooBigErr:
            return "Stream mirror must have max message size >= source.";
        
        case JSMirrorWithSourcesErr:
            return "Stream mirrors can not also contain other sources.";
        
        case JSMirrorWithStartSeqAndTimeErr:
            return "Stream mirrors can not have both start seq and start time configured.";
        
        case JSMirrorWithSubjectFiltersErr:
            return "Stream mirrors can not contain filtered subjects.";
        
        case JSMirrorWithSubjectsErr:
            return "Stream mirrors can not also contain subjects.";
        
        case JSNoAccountErr:
            return "Account not found.";
        
        case JSClusterUnSupportFeatureErr:
            return "Not currently supported in clustered mode.";
        
        case JSNoMessageFoundErr:
            return "No message found.";
        
        case JSNotEmptyRequestErr:
            return "Expected an empty request payload.";
        
        case JSNotEnabledForAccountErr:
            return "JetStream not enabled for account.";
        
        case JSClusterPeerNotMemberErr:
            return "Peer not a member.";
        
        case JSRaftGeneralErr:
            return "General RAFT error.";
        
        case JSRestoreSubscribeFailedErr:
            return "JetStream unable to subscribe to restore snapshot.";
        
        case JSSequenceNotFoundErr:
            return "Sequence not found.";
        
        case JSClusterServerNotMemberErr:
            return "Server is not a member of the cluster.";
        
        case JSSourceConsumerSetupFailedErr:
            return "General source consumer setup failure.";
        
        case JSSourceMaxMessageSizeTooBigErr:
            return "Stream source must have max message size >= target.";
        
        case JSStorageResourcesExceededErr:
            return "Insufficient storage resources available.";
        
        case JSStreamAssignmentErr:
            return "Generic stream assignment error.";
        
        case JSStreamCreateErr:
            return "Generic stream creation error.";
        
        case JSStreamDeleteErr:
            return "General stream deletion error.";
        
        case JSStreamGeneralError:
            return "General stream failure.";
        
        case JSStreamInvalidConfig:
            return "Stream configuration validation error.";
        
        case JSStreamLimitsErr:
            return "General stream limits exceeded error.";
        
        case JSStreamMessageExceedsMaximumErr:
            return "Message size exceeds maximum allowed.";
        
        case JSStreamMirrorNotUpdatableErr:
            return "Mirror configuration can not be updated.";
        
        case JSStreamMismatchErr:
            return "Stream name in subject does not match request.";
        
        case JSStreamMsgDeleteFailed:
            return "Generic message deletion failure error.";
        
        case JSStreamNameExistErr:
            return "Stream name already in use.";
        
        case JSStreamNotFoundErr:
            return "Stream not found.";
        
        case JSStreamNotMatchErr:
            return "Expected stream does not match.";
        
        case JSStreamReplicasNotUpdatableErr:
            return "Replicas configuration can not be updated.";
        
        case JSStreamRestoreErr:
            return "Restore failed.";
        
        case JSStreamSequenceNotMatchErr:
            return "Expected stream sequence does not match.";
        
        case JSStreamSnapshotErr:
            return "Snapshot failed.";
        
        case JSStreamSubjectOverlapErr:
            return "Subjects overlap with an existing stream.";
        
        case JSStreamTemplateCreateErr:
            return "Generic template creation failed.";
        
        case JSStreamTemplateDeleteErr:
            return "Generic stream template deletion failed error.";
        
        case JSStreamTemplateNotFoundErr:
            return "Template not found.";
        
        case JSStreamUpdateErr:
            return "Generic stream update error.";
        
        case JSStreamWrongLastMsgIDErr:
            return "Wrong last msg ID.";
        
        case JSStreamWrongLastSequenceErr:
            return "Wrong last sequence.";
        
        case JSTempStorageFailedErr:
            return "JetStream unable to open temp storage for restore.";
        
        case JSTemplateNameNotMatchSubjectErr:
            return "Template name in subject does not match request.";
        
        case JSStreamReplicasNotSupportedErr:
            return "Replicas > 1 not supported in non-clustered mode.";
        
        case JSPeerRemapErr:
            return "Peer remap failed.";
        
        case JSNotEnabledErr:
            return "JetStream not enabled.";
        
        case JSStreamStoreFailedErr:
            return "Generic error when storing a message failed.";
        
        case JSConsumerConfigRequiredErr:
            return "Consumer config required.";
        
        case JSConsumerDeliverToWildcardsErr:
            return "Consumer deliver subject has wildcards.";
        
        case JSConsumerPushMaxWaitingErr:
            return "Consumer in push mode can not set max waiting.";
        
        case JSConsumerDeliverCycleErr:
            return "Consumer deliver subject forms a cycle.";
        
        case JSConsumerMaxPendingAckPolicyRequiredErr:
            return "Consumer requires ack policy for max ack pending.";
        
        case JSConsumerSmallHeartbeatErr:
            return "Consumer idle heartbeat needs to be >= 100ms.";
        
        case JSConsumerPullRequiresAckErr:
            return "Consumer in pull mode requires explicit ack policy.";
        
        case JSConsumerPullNotDurableErr:
            return "Consumer in pull mode requires a durable name.";
        
        case JSConsumerPullWithRateLimitErr:
            return "Consumer in pull mode can not have rate limit set.";
        
        case JSConsumerMaxWaitingNegativeErr:
            return "Consumer max waiting needs to be positive.";
        
        case JSConsumerHBRequiresPushErr:
            return "Consumer idle heartbeat requires a push based consumer.";
        
        case JSConsumerFCRequiresPushErr:
            return "Consumer flow control requires a push based consumer.";
        
        case JSConsumerDirectRequiresPushErr:
            return "Consumer direct requires a push based consumer.";
        
        case JSConsumerDirectRequiresEphemeralErr:
            return "Consumer direct requires an ephemeral consumer.";
        
        case JSConsumerOnMappedErr:
            return "Consumer direct on a mapped consumer.";
        
        case JSConsumerFilterNotSubsetErr:
            return "Consumer filter subject is not a valid subset of the interest subjects.";
        
        case JSConsumerInvalidPolicyErr:
            return "Generic delivery policy error.";
        
        case JSConsumerInvalidSamplingErr:
            return "Failed to parse consumer sampling configuration.";
        
        case JSStreamInvalidErr:
            return "Stream not valid.";
        
        case JSConsumerWQRequiresExplicitAckErr:
            return "Workqueue stream requires explicit ack.";
        
        case JSConsumerWQMultipleUnfilteredErr:
            return "Multiple non-filtered consumers not allowed on workqueue stream.";
        
        case JSConsumerWQConsumerNotUniqueErr:
            return "Filtered consumer not unique on workqueue stream.";
        
        case JSConsumerWQConsumerNotDeliverAllErr:
            return "Consumer must be deliver all on workqueue stream.";
        
        case JSConsumerNameTooLongErr:
            return "Consumer name is too long.";
        
        case JSConsumerBadDurableNameErr:
            return "Durable name can not contain '.', '*', '>'.";
        
        case JSConsumerStoreFailedErr:
            return "Error creating store for consumer.";
        
        case JSConsumerExistingActiveErr:
            return "Consumer already exists and is still active.";
        
        case JSConsumerReplacementWithDifferentNameErr:
            return "Consumer replacement durable config not the same.";
        
        case JSConsumerDescriptionTooLongErr:
            return "Consumer description is too long.";
        
        case JSConsumerWithFlowControlNeedsHeartbeatsErr:
            return "Consumer with flow control also needs heartbeats.";
        
        case JSStreamSealedErr:
            return "Invalid operation on sealed stream.";
        
        case JSStreamPurgeFailedErr:
            return "Generic stream purge failure.";
        
        case JSStreamRollupFailedErr:
            return "Generic stream rollup failure.";
        
        case JSConsumerInvalidDeliverSubjectErr:
            return "Invalid push consumer deliver subject.";
        
        case JSStreamMaxBytesRequiredErr:
            return "Account requires a stream config to have max bytes set.";
        
        case JSConsumerMaxRequestBatchNegativeErr:
            return "Consumer max request batch needs to be > 0.";
        
        case JSConsumerMaxRequestExpiresToSmallErr:
            return "Consumer max request expires needs to be > 1ms.";
        
        case JSConsumerMaxDeliverBackoffErr:
            return "Max deliver is required to be > length of backoff values.";
        
        case JSStreamInfoMaxSubjectsErr:
            return "Subject details would exceed maximum allowed.";

        case JSStreamOfflineErr:
            return "Stream is offline";

		case JSConsumerOfflineErr:
            return "Consumer is offline";
		
        case JSNoLimitsErr:
            return "No JetStream default or applicable tiered limit present";
		
        case JSConsumerMaxPendingAckExcessErrF:
            return "Consumer max ack pending exceeds system limit of {limit}";
		
        case JSStreamMaxStreamBytesExceeded:
           return "Stream max bytes exceeds account limit max stream bytes";
		
        case JSStreamMoveAndScaleErr:
            return "Can not move and scale a stream in a single update";
		
        case JSStreamMoveInProgressF:
            return "Stream move already in progress: {msg}";

		case JSConsumerMaxRequestBatchExceededF:
            return "Consumer max request batch exceeds server limit of {limit}";
		
        case JSConsumerReplicasExceedsStream:
          return "Consumer config replica count exceeds parent stream";
		
        case JSConsumerNameContainsPathSeparatorsErr:
            return "Consumer name can not contain path separators";
		
        case JSStreamNameContainsPathSeparatorsErr:
            return "Stream name can not contain path separators";
		
        case JSStreamMoveNotInProgress:
            return "Stream move not in progress";
		
        case JSStreamNameExistRestoreFailedErr:
            return "Stream name already in use, cannot restore";
		
        case JSConsumerCreateFilterSubjectMismatchErr:
            return "Consumer create request did not match filtered subject from create subject";
		
        case JSConsumerCreateDurableAndNameMismatch:
            return "Consumer Durable and Name have to be equal if both are provided";
		
        case JSReplicasCountCannotBeNegative:
            return "Replicas count cannot be negative";

        default:
            return "Unknown error.";
    }

    #pragma clang diagnostic pop
    #pragma GCC diagnostic pop
}



void jetstream_message_queue_publisher_impl::_send(std::string_view sv) {

    /* Aliases */
    using unique_jsPubAck_ptr_t = std::unique_ptr<jsPubAck, decltype(&jsPubAck_Destroy)>;
    
    natsStatus status = NATS_OK;
    jsErrCode jerr = static_cast<jsErrCode>(0);
    unique_jsPubAck_ptr_t jsPubAck_ptr(nullptr, jsPubAck_Destroy);

    /* TODO see if we can avoid copy here on each call */
    char msg_id_as_cstr[_client_id.length() + UINT64_BUFSIZE + 1];
    std::ranges::copy(_client_id, msg_id_as_cstr);

    char *out_ptr = msg_id_as_cstr + _client_id.length();
    (*out_ptr++) = '-';

    auto [end_ptr2, ec2] = std::to_chars(out_ptr, 
                                    msg_id_as_cstr+sizeof(msg_id_as_cstr), 
                                    _msg_id.fetch_add(1, std::memory_order_relaxed));
    *end_ptr2 = '\0';


    /* TODO not thread safe, need to fix */
    _jsPubOpts.MsgId = msg_id_as_cstr;

    std::cout << "msg id: " << _jsPubOpts.MsgId << std::endl;


    do {    
        {
            jsPubAck *pa = nullptr;

            status = js_Publish(&pa, 
                                _jsCtx_ptr.get(), 
                                _subject.c_str(), 
                                sv.data(), 
                                sv.length(), 
                                &_jsPubOpts, 
                                &jerr);

            jsPubAck_ptr.reset(pa);
        }



        /* TODO not sure if this is a necessary check */
        /* Check for duplicate msg error */
        if(jsPubAck_ptr && jsPubAck_ptr->Duplicate) {
            std::clog << "Publish NATS error: " 
                            << natsStatus_GetText(status) 
                            << " JS err: " 
                            << _jsError_GetText(jerr)
                            << " Duplicate: " 
                            << jsPubAck_ptr->Duplicate
                            << std::endl;
            status = NATS_OK;
            continue;
        }


        /* Process the result of the operation */
        switch(status) {

            /* Success */
            case NATS_OK:
                break;

            /* Generic NATS ERROR */
            case NATS_ERR:

                /* TODO print statement to find errors we should handle */
                std::clog << "Publish NATS error: " 
                            << natsStatus_GetText(status) 
                            << " JS err: " 
                            << _jsError_GetText(jerr)
                            << std::endl;
                            
                /* Process js errors */
                switch(jerr) {

                    /* Stream may be full so wait and resend */
                    case JSStreamStoreFailedErr:
                        nats_Sleep(1000);
                        break;  
                
                
                    default: 
                        throw std::runtime_error(
                            std::string("Publish NATS error: ")+
                            natsStatus_GetText(status)+ 
                            " JS err: " +
                            _jsError_GetText(jerr));
                        break;
                }

                break;

            /* For timeout explicitly try again */
            case NATS_TIMEOUT:
                /* TODO print statement to find errors we should handle */
                std::clog << "Publish NATS error: " 
                            << natsStatus_GetText(status) 
                            << " JS err: " 
                            << _jsError_GetText(jerr)
                            << std::endl;
                continue;

            /* No servers are reachable so let's try again in 5 seconds */
            case NATS_NO_RESPONDERS:
                /* TODO print statement to find errors we should handle */
                std::clog << "Publish NATS error: " 
                            << natsStatus_GetText(status) 
                            << " JS err: " 
                            << _jsError_GetText(jerr)
                            << std::endl;
                nats_Sleep(5000);
                continue;

            /* Other errors should be an error */
            case NATS_PROTOCOL_ERROR:
            case NATS_IO_ERROR:
            case NATS_LINE_TOO_LONG:
            case NATS_CONNECTION_CLOSED:
            case NATS_NO_SERVER:
            case NATS_STALE_CONNECTION:
            case NATS_SECURE_CONNECTION_WANTED:
            case NATS_SECURE_CONNECTION_REQUIRED:
            case NATS_CONNECTION_DISCONNECTED:
            case NATS_CONNECTION_AUTH_FAILED:
            case NATS_NOT_PERMITTED:
            case NATS_NOT_FOUND:
            case NATS_ADDRESS_MISSING:
            case NATS_INVALID_SUBJECT:
            case NATS_INVALID_ARG:
            case NATS_INVALID_SUBSCRIPTION:
            case NATS_INVALID_TIMEOUT:
            case NATS_ILLEGAL_STATE:
            case NATS_SLOW_CONSUMER:
            case NATS_MAX_PAYLOAD:
            case NATS_MAX_DELIVERED_MSGS:
            case NATS_INSUFFICIENT_BUFFER:
            case NATS_NO_MEMORY:
            case NATS_SYS_ERROR:
            case NATS_FAILED_TO_INITIALIZE:
            case NATS_NOT_INITIALIZED:
            case NATS_SSL_ERROR:
            case NATS_NO_SERVER_SUPPORT:
            case NATS_NOT_YET_CONNECTED:
            case NATS_DRAINING:
            case NATS_INVALID_QUEUE_NAME:
            case NATS_MISMATCH:
            case NATS_MISSED_HEARTBEAT:

                /* TODO print statement to find errors we should handle */
                std::clog << "Publish NATS error: " 
                            << natsStatus_GetText(status) 
                            << " JS err: " 
                            << _jsError_GetText(jerr)
                            << " Duplicate: " 
                            << jsPubAck_ptr->Duplicate
                            << std::endl;

                /* TODO not sure if this is a necessary check */
                /* Check for duplicate msg error */
                if(jsPubAck_ptr && jsPubAck_ptr->Duplicate) {
                    status = NATS_OK;
                    continue;
                }

                /* Process js errors */
                switch(jerr) {

                    /* Other errors should be an error */
                    default:
                        throw std::runtime_error(
                            std::string("Publish NATS error: ")+
                            natsStatus_GetText(status)+ 
                            " JS err: " +
                            _jsError_GetText(jerr));
                }
        }

    } while(status != NATS_OK);

    /* Flush the connection */
    natsConnection_Flush(_conn_ptr.get());
}


void jetstream_message_queue_subscriber_impl::_receive(
    const unique_natsMsgList_ptr_t &msgListPtr) {
     
    natsStatus status = static_cast<natsStatus>(~NATS_OK);
    jsErrCode jerr    = static_cast<jsErrCode>(0);

    while(status != NATS_OK) {
        status = natsSubscription_Fetch(msgListPtr.get(), 
                                        _sub_ptr.get(), 
                                        1, 
                                        5000, 
                                        &jerr);

        if(status != NATS_OK) {

            /* TODO print statement to find errors we should handle */
            std::clog << "Subscriber NATS error: " 
                        << natsStatus_GetText(status) 
                        << " JS err: " 
                        << _jsError_GetText(jerr)
                        << std::endl;
        }
    }

    natsMsg *msg_ptr = msgListPtr->Msgs[0];
   
    /* Acknowledge the message */
    status = natsMsg_Ack(msg_ptr, nullptr);

    if(status != NATS_OK) {
        throw std::runtime_error(std::string("Subscriber NATS error: ")+
                    natsStatus_GetText(status));
    }  

    natsConnection_Flush(_conn_ptr.get());
};


// static std::tuple<std::string, std::string, std::string> 
// url_parse(const std::string_view url) {

//     auto end = url.find('.');

//     if(end == std::string::npos) {
//         throw std::runtime_error(std::string("Ill formed url ")+url.data());
//     }

//     std::string stream_name(url, 0, end);


//     auto pos = end+1;
//     end = url.find('.', pos);

//     if(end == std::string::npos) {
//         throw std::runtime_error(std::string("Ill formed url ")+url.data());
//     }

//     std::string consumer_name(url, pos, end-pos);

//     ++end;
    
//     if(url.length()-end <= 0) {
//         throw std::runtime_error(std::string("Ill formed url ")+url.data());
//     }

//     std::string subject(url, end, url.length()-end);

    
//     /* Return the 3 components as structured binding */
//     return { stream_name, consumer_name, subject };
// }

auto jetstream_messaging_service_impl::create_queue_publisher(
        std::string_view stream, 
        std::string_view consumer, 
        std::string_view subject) -> jetstream_message_queue_publisher_impl {

    using unique_jsStreamInfo_ptr_t = std::unique_ptr<jsStreamInfo, decltype(&jsStreamInfo_Destroy)>;
    
    natsStatus status = static_cast<natsStatus>(~NATS_OK);


    /* Timeout lengths to use by default */
    auto timeouts = {5, 30, 60, 120};

    for(auto timeout = timeouts.begin(); 
        status != NATS_OK and timeout != timeouts.end(); ++timeout) {
    
        jsStreamInfo *si = nullptr;
        jsErrCode jerr   = static_cast<jsErrCode>(0);

        /* Checks for the existance of the stream */
        status = js_GetStreamInfo(&si, 
                                    _jsCtx_ptr.get(), 
                                    stream.data(), 
                                    nullptr, 
                                    &jerr);

        /* Assign stream info to a unique pointer so it will be 
            * released regardless of how we exit. */
        unique_jsStreamInfo_ptr_t si_ptr(si, jsStreamInfo_Destroy);

        /* Process the result of the operation */
        switch(status) {

            /* Success, stream found */
            case NATS_OK:
                break;

            /* Failure, stream not found */
            case NATS_NOT_FOUND:
                throw std::runtime_error(std::string("Create_queue_publisher: ") 
                                        + natsStatus_GetText(status));
                break;

            /* For timeout and no responders, let's wait and 
                * then attempt to join */
            case NATS_TIMEOUT:
            case NATS_NO_RESPONDERS:
                nats_Sleep(*timeout * 1000);
                break;

            /* Default error handler */
            default:
                throw std::runtime_error(
                    std::string("Create_queue_publisher: ")+
                    natsStatus_GetText(status) + 
                    ((jerr != 0) ? (std::string(" JS err: ") + _jsError_GetText(jerr)) : ""));
        }     
    }

    /* Timeout occurred */
    if(status == NATS_TIMEOUT || status == NATS_NO_RESPONDERS) {
        throw std::runtime_error(
                    std::string("Create_queue_publisher: ") + 
                    "Timeout or no responses");
    }

    /* Generate a client id */
    boost::uuids::uuid client_uuid = boost::uuids::random_generator()();

    /* Create the message queue publisher */
    return jetstream_message_queue_publisher_impl(
                stream,
                consumer,
                subject,
                _conn_ptr, 
                _jsCtx_ptr, 
                boost::uuids::to_string(client_uuid));
}
        
auto jetstream_messaging_service_impl::create_queue_subscriber(
        std::string_view stream, 
        std::string_view consumer, 
        std::string_view subject) -> jetstream_message_queue_subscriber_impl {

    /* Aliases */
    using unique_jsStreamInfo_ptr_t = std::unique_ptr<jsStreamInfo, decltype(&jsStreamInfo_Destroy)>;
    using unique_natsSubscription_ptr_t = std::unique_ptr<natsSubscription, decltype(&natsSubscription_Destroy)>;
    using unique_jsConsumerInfo_ptr_t = std::unique_ptr<jsConsumerInfo, decltype(&jsConsumerInfo_Destroy)>;

    
    natsStatus status = static_cast<natsStatus>(~NATS_OK);
    jsErrCode jerr    = static_cast<jsErrCode>(0);

    /* Timeout lengths to use by default */
    auto timeouts = {5, 30, 60, 120};


    for(auto timeout = timeouts.begin(); 
        status != NATS_OK and timeout != timeouts.end(); ++timeout) {
        
        jsStreamInfo *si = nullptr;
        
        /* Checks for the existance of the stream */
        status = js_GetStreamInfo(&si, 
                                    _jsCtx_ptr.get(), 
                                    stream.data(), 
                                    nullptr, 
                                    &jerr);

        /* Assign stream info to a unique pointer so it will be 
            * released regardless of how we exit. */                
        unique_jsStreamInfo_ptr_t si_ptr(si, jsStreamInfo_Destroy);

        /* Process the result of the operation */
        switch(status) {

            /* Success */
            case NATS_OK:
                break;

            /* Failure, stream not found */
            case NATS_NOT_FOUND:
                throw std::runtime_error(
                    std::string("Create_queue_subscriber: ") 
                                + natsStatus_GetText(status));
                break;

            /* For timeout and no responders, let's wait and 
             * then attempt to join */
            case NATS_TIMEOUT:
            case NATS_NO_RESPONDERS:
                nats_Sleep(*timeout * 1000);
                break;

            /* Default error handler */
            default:
                throw std::runtime_error(
                    std::string("Create_queue_subscriber: ")+
                    natsStatus_GetText(status) + 
                    ((jerr != 0) ? (std::string(" JS err: ") + _jsError_GetText(jerr)) : ""));
                break;
        }
    } 

    /* Timeout occurred */
    if(status == NATS_TIMEOUT || status == NATS_NO_RESPONDERS) {
        throw std::runtime_error(
                    std::string("Create_queue_subscriber: ") + 
                    "Timeout or no responses");
    }


    /* Retrieve info for the consumer and verify it exists. */    
    jsConsumerInfo *jsConsumerInfo = nullptr;

    status = js_GetConsumerInfo(&jsConsumerInfo, 
                                _jsCtx_ptr.get(),
                                stream.data(), 
                                consumer.data(),
                                nullptr,
                                &jerr); 

    if(status != NATS_OK) {
        throw std::runtime_error(
                std::string("Create_queue_subscriber: ")+
                natsStatus_GetText(status) + 
                ((jerr != 0) ? (std::string(" JS err: ") + _jsError_GetText(jerr)) : ""));
    }

    /* Assign the pointer to the unique ptr for deletion on method exit. */
    unique_jsConsumerInfo_ptr_t consumerInfo_ptr(jsConsumerInfo, 
                                                 jsConsumerInfo_Destroy);

    natsSubscription *subscription = nullptr;

    /* Create the pull subscriber */
    status = js_PullSubscribe(&subscription, 
                            _jsCtx_ptr.get(), 
                            subject.data(), 
                            consumer.data(), 
                            nullptr, 
                            nullptr, 
                            &jerr);

    if(status != NATS_OK) {
        throw std::runtime_error(std::string("Consumer NATS error: ")+
                natsStatus_GetText(status)+ 
                ((jerr != 0) ? std::string(" JS err: ") +_jsError_GetText(jerr) : ""));
    }

    /* Assign to unique pointer so it will get destroyed */
    unique_natsSubscription_ptr_t sub_ptr(subscription, 
                                            natsSubscription_Destroy);
    
    /* Return an instance of the jetstream message queue wrapper */
    return jetstream_message_queue_subscriber_impl(
                stream,
                consumer,
                subject, 
                _conn_ptr, 
                std::move(sub_ptr));
}   

jetstream_messaging_service_impl _create_messaging_service(
    std::string_view urls) {

    /* Unique_ptr aliases */
    using unique_natsOptions_ptr_t = std::unique_ptr<natsOptions, decltype(&natsOptions_Destroy)>;
    using unique_natsConnection_ptr_t = std::unique_ptr<natsConnection, decltype(&natsConnection_Destroy)>;
    using unique_jsCTX_ptr_t = std::unique_ptr<jsCtx, decltype(&jsCtx_Destroy)>;

    unique_natsOptions_ptr_t opts_ptr(nullptr, natsOptions_Destroy);
    unique_natsConnection_ptr_t conn_ptr(nullptr, natsConnection_Destroy);
    unique_jsCTX_ptr_t jsCTX_ptr(nullptr, jsCtx_Destroy);
    natsStatus status;
    jsOptions jsOpts;
     

    /* Create the natsOption */
    {
        natsOptions *opts = nullptr;

        status = natsOptions_Create(&opts);

        if(status != NATS_OK) {
            throw std::runtime_error(std::string("NATS error: ")+
                                natsStatus_GetText(status));
        }

        opts_ptr.reset(opts);
    }

        /* Set the options for reconnects */
    natsOptions_SetAllowReconnect(opts_ptr.get(), true);
    natsOptions_SetMaxReconnect(opts_ptr.get(), INT_MAX);
    natsOptions_SetDisconnectedCB(opts_ptr.get(), 
        [](natsConnection *conn, void *closure){}, nullptr);
    natsOptions_SetReconnectedCB(opts_ptr.get(), 
        [](natsConnection *conn, void *closure){}, nullptr);
    natsOptions_SetCustomReconnectDelay(opts_ptr.get(), 
        [](natsConnection *conn, int attempts, void *closure)->int64_t { return 10000; }, nullptr);

    /* Set a ping interval between the client and server to ensure server is up. */
    status = natsOptions_SetPingInterval(opts_ptr.get(), 1000);

    if(status != NATS_OK) {
        throw std::runtime_error(std::string("NATS error: ")+
                            natsStatus_GetText(status));
    }

    /* Number of pings until we fail a server */
    status = natsOptions_SetMaxPingsOut(opts_ptr.get(), 3);

    if(status != NATS_OK) {
        throw std::runtime_error(std::string("NATS error: ")+
                            natsStatus_GetText(status));
    }
    
     /* Build a list server urls from the main url */
    std::vector<std::string> split_urls;

    for(std::string_view::size_type start = 0, end;
        start != std::string_view::npos && (end = urls.find(',', start)) != start;
        start = end + ((end != std::string_view::npos) ? 1 : 0)) {

        split_urls.emplace_back(urls.substr(start, end-start));
    }
    
    const char *servers[split_urls.size()];

    for(auto i=0; i<split_urls.size(); ++i) {
        servers[i] = split_urls[i].c_str();
    }

    /* Provide the servers to NATS */
    status = natsOptions_SetServers(opts_ptr.get(), servers, split_urls.size());

    if(status != NATS_OK) {
        throw std::runtime_error(std::string("NATS error: ")+
                            natsStatus_GetText(status));
    }

    /* TODO Remove this in production, this is for testing server failure */
    status = natsOptions_SetNoRandomize(opts_ptr.get(), false);

    if(status != NATS_OK) {
        throw std::runtime_error(std::string("NATS error: ")+
                            natsStatus_GetText(status));
    }

    /* Create and connect the nats connection */
    {
        natsConnection *conn = nullptr;

        
        /* Connect to a NATS servers */
        status = natsConnection_Connect(&conn, opts_ptr.get());

        if(status != NATS_OK) {
            throw std::runtime_error(std::string("NATS error: ")+
                                natsStatus_GetText(status));
        }

        conn_ptr.reset(conn);

        printf("connected\n");
    }

    /* Initialize a struct for JetStream options */
    status = jsOptions_Init(&jsOpts);

    if(status != NATS_OK) {
        throw std::runtime_error(std::string("NATS error: ")+
                            natsStatus_GetText(status));
    }

    jsOpts.Wait = 5000;
    
    {
        jsCtx *jsctx = nullptr;

        /* Create the JetStream context overlay on top of NATS */
        status = natsConnection_JetStream(&jsctx, conn_ptr.get(), &jsOpts);
        
        if(status != NATS_OK) {
            throw std::runtime_error(std::string("NATS error: ")+
                                natsStatus_GetText(status));
        }

        jsCTX_ptr.reset(jsctx);

        printf("Jetstream connected\n");
    }

    return jetstream_messaging_service_impl(
                std::move(conn_ptr), 
                std::move(jsCTX_ptr));
};


