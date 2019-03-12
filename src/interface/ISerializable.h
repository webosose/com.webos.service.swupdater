/* @@@LICENSE
 *
 * Copyright (c) 2019 LG Electronics, Inc.
 *
 * Confidential computer software. Valid license from LG required for
 * possession, use or copying. Consistent with FAR 12.211 and 12.212,
 * Commercial Computer Software, Computer Software Documentation, and
 * Technical Data for Commercial Items are licensed to the U.S. Government
 * under vendor's standard commercial license.
 *
 * LICENSE@@@
 */

#ifndef INTERFACE_ISERIALIZABLE_H_
#define INTERFACE_ISERIALIZABLE_H_

#include <pbnjson.hpp>

using namespace pbnjson;

class ISerializable {
public:
    ISerializable() {};
    virtual ~ISerializable() {};

    virtual bool fromJson(const JValue& json) = 0;
    virtual bool toJson(JValue& json) = 0;
    virtual void printDebug() = 0;

};

#endif /* INTERFACE_ISERIALIZABLE_H_ */
