//
//  _   _         ______    _ _ _   _             _ _ _
// | \ | |       |  ____|  | (_) | (_)           | | | |
// |  \| | ___   | |__   __| |_| |_ _ _ __   __ _| | | |
// | . ` |/ _ \  |  __| / _` | | __| | '_ \ / _` | | | |
// | |\  | (_) | | |___| (_| | | |_| | | | | (_| |_|_|_|
// |_| \_|\___/  |______\__,_|_|\__|_|_| |_|\__, (_|_|_)
//                                           __/ |
//                                          |___/
// 
// This file is auto-generated. Do not edit manually
// 
// Copyright 2013-2019 Automatak, LLC
// 
// Licensed to Green Energy Corp (www.greenenergycorp.com) and Automatak
// LLC (www.automatak.com) under one or more contributor license agreements.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership. Green Energy Corp and Automatak LLC license
// this file to you under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You may obtain
// a copy of the License at:
// 
//   http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "opendnp3/gen/KeyWrapAlgorithm.h"
#include <stdexcept>

namespace opendnp3 {

uint8_t KeyWrapAlgorithmSpec::to_type(KeyWrapAlgorithm arg)
{
  return static_cast<uint8_t>(arg);
}

KeyWrapAlgorithm KeyWrapAlgorithmSpec::from_type(uint8_t arg)
{
  switch(arg)
  {
    case(0x1):
      return KeyWrapAlgorithm::AES_128;
    case(0x2):
      return KeyWrapAlgorithm::AES_256;
    default:
      return KeyWrapAlgorithm::UNDEFINED;
  }
}

char const* KeyWrapAlgorithmSpec::to_string(KeyWrapAlgorithm arg)
{
  switch(arg)
  {
    case(KeyWrapAlgorithm::AES_128):
      return "AES_128";
    case(KeyWrapAlgorithm::AES_256):
      return "AES_256";
    default:
      return "UNDEFINED";
  }
}

char const* KeyWrapAlgorithmSpec::to_human_string(KeyWrapAlgorithm arg)
{
  switch(arg)
  {
    case(KeyWrapAlgorithm::AES_128):
      return "AES_128";
    case(KeyWrapAlgorithm::AES_256):
      return "AES_256";
    default:
      return "UNDEFINED";
  }
}

KeyWrapAlgorithm KeyWrapAlgorithmSpec::from_string(const std::string& arg)
{
  if(arg == "AES_128") return KeyWrapAlgorithm::AES_128;
  if(arg == "AES_256") return KeyWrapAlgorithm::AES_256;
  else return KeyWrapAlgorithm::UNDEFINED;
}


}
