/*
 * Copyright (c) 2013-2019 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <QString>
#include "de_web_plugin.h"
#include "de_web_plugin_private.h"
#include "json.h"

int DeRestPluginPrivate::handleAttributesApi(const ApiRequest &req, ApiResponse &rsp)
{
    if (req.path[2] != QLatin1String("attributes"))
    {
        return REQ_NOT_HANDLED;
    }

    // GET /api/<apikey>/attributes
    if ((req.path.size() == 4) && (req.hdr.method() == "POST"))
    {
        return writeAttribute(req, rsp);
    }
    return REQ_NOT_HANDLED;
}

/*! GET /api/<apikey>/attributes
    \return REQ_READY_SEND
            REQ_NOT_HANDLED
 */
int DeRestPluginPrivate::writeAttribute(const ApiRequest &req, ApiResponse &rsp)
{
    DBG_Assert(req.path.size() == 4);
    const QString &id = req.path[3];
    bool ok;
    QVariant var = Json::parse(req.content, ok);
    QVariantMap map = var.toMap();
    if (!ok || map.isEmpty()) {
        rsp.list.append(errorToMap(ERR_INVALID_JSON, QString("/attributes/%1").arg(id), QString("body contains invalid JSON")));
        rsp.httpStatus = HttpStatusBadRequest;
        return REQ_READY_SEND;
    }
    if (!map.contains("key")) {
        rsp.list.append(errorToMap(ERR_INVALID_VALUE, QString("/attributes/%1").arg(id), QString("key is required")));
        rsp.httpStatus = HttpStatusNotFound;
        return REQ_READY_SEND;
    }
    if (!map.contains("value")) {
        rsp.list.append(errorToMap(ERR_INVALID_VALUE, QString("/attributes/%1").arg(id), QString("value is required")));
        rsp.httpStatus = HttpStatusNotFound;
        return REQ_READY_SEND;
    }

    const int endpoint = 1;
    const int mfcode = 0x115f;
    const uint16_t key = map["key"].toInt(&ok);
    const uint8_t value = map["value"].toInt(&ok);
    DBG_Printf(DBG_INFO, "id: %s\n", qPrintable(id));
    DBG_Printf(DBG_INFO, "key: %d\n", key);
    DBG_Printf(DBG_INFO, "value: %d\n", value);
    
    RestNodeBase *node = getSensorNodeForId(id);
    DBG_Printf(DBG_INFO, "node unique id: %s\n", qPrintable(node->uniqueId()));
    
    deCONZ::ZclAttribute attr(key, deCONZ::Zcl8BitUint, "dec", deCONZ::ZclReadWrite, true);
    attr.setValue((quint64)value);
    if (writeAttribute(node, endpoint, BASIC_CLUSTER_ID, attr, mfcode)) {
        DBG_Printf(DBG_INFO, "Write successful\n");
        rsp.list.append(QLatin1String("success"));
    } else {
        DBG_Printf(DBG_INFO, "Write unsuccessful\n");
        rsp.list.append(QLatin1String("failure"));
    }
    rsp.httpStatus = HttpStatusOk;
    return REQ_READY_SEND;
}
