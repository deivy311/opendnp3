/**
 * Licensed to Green Energy Corp (www.greenenergycorp.com) under one or
 * more contributor license agreements. See the NOTICE file distributed
 * with this work for additional information regarding copyright ownership.
 * Green Energy Corp licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This project was forked on 01/01/2013 by Automatak, LLC and modifications
 * may have been made to this file. Automatak, LLC licenses these modifications
 * to you under the terms of the License.
 */

#include "MasterAuthContext.h"

#include <opendnp3/LogLevels.h>
#include <opendnp3/master/MasterContext.h>

#include <openpal/logging/LogMacros.h>

#include "secauth/master/AuthResponseHandler.h"
#include "opendnp3/app/parsing/APDUParser.h"

#include "secauth/Crypto.h"
#include "secauth/HMACProvider.h"

using namespace openpal;
using namespace opendnp3;

namespace secauth
{

MAuthContext::MAuthContext(
		openpal::IExecutor& executor,
		openpal::LogRoot& root,
		opendnp3::ILowerLayer& lower,
		opendnp3::ISOEHandler& SOEHandler,
		opendnp3::IMasterApplication& application,
		const opendnp3::MasterParams& params,
		opendnp3::ITaskLock& taskLock,
		openpal::ICryptoProvider& crypto,
		IMasterUser& user
	) : 
	MContext(executor, root, lower, SOEHandler, application, params, taskLock),
	msstate(application, executor, crypto, user),
	sessionKeyTask(application, TimeDuration::Seconds(5), logger, user.GetUser(), msstate)
{

}

bool MAuthContext::GoOnline()
{
	auto ret = MContext::GoOnline();

	if (ret)
	{
		// add the session key task to the scheduler
		this->scheduler.Schedule(openpal::ManagedPtr<IMasterTask>::WrapperOnly(&sessionKeyTask));
	}
		
	return ret;
}

bool MAuthContext::GoOffline()
{
	return MContext::GoOffline();
}

void MAuthContext::OnReceive(const openpal::ReadBufferView& apdu, const opendnp3::APDUResponseHeader& header, const openpal::ReadBufferView& objects)
{	
	switch (header.function)
	{
	case(FunctionCode::AUTH_RESPONSE) :
		this->OnReceiveAuthResponse(apdu, header, objects);
		break;
	case(FunctionCode::RESPONSE) :
		this->ProcessResponse(header, objects);
		break;
	case(FunctionCode::UNSOLICITED_RESPONSE) :
		this->ProcessUnsolicitedResponse(header, objects);
		break;
	default:
		FORMAT_LOG_BLOCK(this->logger, opendnp3::flags::WARN, "Ignoring unsupported function code: %s", FunctionCodeToString(header.function));
		break;
	}	
}

void MAuthContext::RecordLastRequest(const openpal::ReadBufferView& apdu)
{
	this->lastRequest = apdu;
}

void MAuthContext::OnReceiveAuthResponse(const openpal::ReadBufferView& apdu, const opendnp3::APDUResponseHeader& header, const openpal::ReadBufferView& objects)
{
	// need to determine the context of the auth response
	
	if (this->pState->ExpectingResponse())
	{
		// an auth-based task is running and needs to receive this directly
		if (this->pActiveTask->AcceptsFunction(FunctionCode::AUTH_RESPONSE))
		{
			this->ProcessResponse(header, objects);
		}
		else
		{
			AuthResponseHandler handler(apdu, header, *this);
			APDUParser::Parse(objects, handler, this->logger);
		}		
	}
	else
	{
		SIMPLE_LOG_BLOCK(this->logger, flags::WARN, "Ignoring AuthResponse"); // TODO - better error message?
	}

}

void  MAuthContext::OnAuthChallenge(const openpal::ReadBufferView& apdu, const opendnp3::APDUHeader& header, const opendnp3::Group120Var1& challenge)
{
	if (this->isSending)
	{
		SIMPLE_LOG_BLOCK(this->logger, flags::WARN, "Ignoring authentication challenge while transmitting");
		return;
	}

	if (!AuthConstants::ChallengeDataSizeWithinLimits(challenge.Size()))
	{
		FORMAT_LOG_BLOCK(this->logger, flags::WARN, "Challenge data size outside of limits: %u", challenge.Size());
		return;
	}

	// lookup the session keys
	SessionKeysView keys;
	if (msstate.session.GetKeys(keys) != KeyStatus::OK)
	{		
		SIMPLE_LOG_BLOCK(this->logger, flags::WARN, "Session for default user is not valid");
		return;
	}

	HMACMode hmacMode;
	if (!Crypto::TryGetHMACMode(challenge.hmacAlgo, hmacMode))
	{
		FORMAT_LOG_BLOCK(this->logger, flags::WARN, "Outstation requested unsupported HMAC type: %s", HMACTypeToString(challenge.hmacAlgo));
		return;
	}	

	HMACProvider hmacProvider(*msstate.pCrypto, hmacMode);

	std::error_code ec;
	auto hmacValue = hmacProvider.Compute(keys.controlKey, { apdu, lastRequest }, ec);

	if (ec)
	{
		SIMPLE_LOG_BLOCK(this->logger, flags::ERR, "Unable to calculate HMAC value");
		return;
	}
		
	Group120Var2 challengeReply;
	challengeReply.challengeSeqNum = challenge.challengeSeqNum;
	challengeReply.userNum = msstate.pUser->GetUser().GetId();
	challengeReply.hmacValue = hmacValue;
	
	APDURequest reply(msstate.challengeReplyBuffer.GetWriteBufferView());
	reply.SetFunction(FunctionCode::AUTH_REQUEST);
	reply.SetControl(AppControlField::Request(header.control.SEQ));
	
	if (!reply.GetWriter().WriteFreeFormat(challengeReply))
	{
		SIMPLE_LOG_BLOCK(this->logger, flags::ERR, "Unable to write challenge reply");
		return;
	}

	this->Transmit(reply.ToReadOnly());
}

void MAuthContext::OnAuthError(const openpal::ReadBufferView& apdu, const opendnp3::APDUHeader& header, const opendnp3::Group120Var7& error)
{
	FORMAT_LOG_BLOCK(this->logger, flags::WARN,
		"Received auth error from outstation w/ code: %s",
		AuthErrorCodeToString(error.errorCode)
	);		

	// TODO - invalidate the session keys for the user?
}

}
